// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRMagazine.h"

#include "TacticalCollisionProfiles.h"
#include "TacticalTraceChannels.h"
#include "Libraries/TVRFunctionLibrary.h"
#include "Player/TVREquipmentPoint.h"
#include "Player/TVRGraspingHand.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/TVRGunWithChild.h"
#include "Weapon/Component/TVRMagazineWell.h"
#include "Weapon/Component/TVRMagWellComponent.h"

ATVRMagazine::ATVRMagazine(const FObjectInitializer& OI) : Super(OI)
{
    GetStaticMeshComponent()->SetCollisionProfileName(COLLISION_WEAPON);
    GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    
    GripSlot = CreateDefaultSubobject<USceneComponent>(FName("GripSlot"));
    GripSlot->SetupAttachment(GetStaticMeshComponent());
    
    AttachOrigin = CreateDefaultSubobject<USceneComponent>(FName("AttachOrigin"));
    AttachOrigin->SetupAttachment(GetStaticMeshComponent());

    MagazineCollider = CreateDefaultSubobject<UBoxComponent>(FName("MagazineCollider"));
    MagazineCollider->SetupAttachment(GetStaticMeshComponent());
    MagazineCollider->SetCollisionProfileName(COLLISION_MAGAZINE_INSERT);
    MagazineCollider->InitBoxExtent(FVector(2.8f, 0.8f, 0.8f));
    MagazineCollider->ShapeColor = FColor::Turquoise;

	DropCollider = CreateDefaultSubobject<UBoxComponent>(FName("DropCollider"));
    DropCollider->SetupAttachment(GetStaticMeshComponent());
    DropCollider->SetCollisionProfileName(COLLISION_NO_COLLISION);
    DropCollider->InitBoxExtent(FVector(1.5f, 2.f, 2.f));
    DropCollider->ShapeColor = FColor::Green;

	RoundsInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>("RoundsInstances");
    RoundsInstances->SetupAttachment(GetStaticMeshComponent());
	RoundsInstances->SetCollisionProfileName(COLLISION_NO_COLLISION);
	RoundsInstances->InstanceStartCullDistance = 100;
	RoundsInstances->InstanceEndCullDistance = 200;
	
	FollowerMesh = CreateDefaultSubobject<UStaticMeshComponent>("Follower");
	FollowerMesh->SetupAttachment(GetStaticMeshComponent());
	FollowerMesh->SetCollisionProfileName(COLLISION_NO_COLLISION);

    VRGripInterfaceSettings.bAllowMultipleGrips = false;
    VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_None;

	AttachedMagWell = nullptr;
    AmmoCapacity = 10;
    CurrentAmmo = AmmoCapacity;
    MagInsertPercentage = 0.f;
	LimitDisplayAmmo = -1;

	bNotFull = false;
	RoundRadius = 0.5f;
	CurveRadius = 0.f;
	CurveStartIdx = 100;
	StackSlope = 0.f;
	RoundScale = FVector::OneVector;

	FollowerOffset = FVector::ZeroVector;
	FollowerMorphBias = 0.f;
	FollowerMorphSlope = 0.1f;

	bSwitchLROrder = false;
	bDoubleStack = true;

	HandSocket = nullptr;
	CartridgeType = nullptr;
}

void ATVRMagazine::BeginPlay()
{
    Super::BeginPlay();
	const float InitAmmo = bNotFull ? CurrentAmmo : AmmoCapacity;
	CurrentAmmo = -1; // little hack to force update of instances and other components
	SetAmmo(InitAmmo);

	const FVector COMWorld = GetStaticMeshComponent()->GetCenterOfMass();
	SavedCenterOfMass = GetActorTransform().InverseTransformPosition(COMWorld);

	MagazineMeshes.Empty();
	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);
	for(UStaticMeshComponent* LoopMesh : Meshes)
	{
		if(LoopMesh && LoopMesh->GetCollisionProfileName() == FName("Weapon"))
		{
			MagazineMeshes.AddUnique(LoopMesh);
		}
	}
	TArray<UHandSocketComponent*> HandSockets;
	GetComponents<UHandSocketComponent>(HandSockets);
	if(HandSockets.IsValidIndex(0))
	{
		HandSocket = HandSockets[0];
	}
}

void ATVRMagazine::Destroyed()
{
	if(IsInserted())
	{
		if(const auto MagWell = Cast<UTVRMagWellComponent>(AttachedMagWell))
		{
			MagWell->OnMagDestroyed();
		}
		else if(const auto MagWell2 = Cast<UTVRMagazineWell>(AttachedMagWell))
		{
			MagWell2->OnMagDestroyed();
		}
	}
	OnMagReleaseReleased();
	Super::Destroyed();
}

void ATVRMagazine::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(GetRoundsComponent())
	{
		GetRoundsComponent()->ClearInstances();
	}
	UpdateRoundInstances();
	UpdateFollowerLocation();
}

void ATVRMagazine::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot,
                                                         bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName,
                                                         UGripMotionControllerComponent* CallingController, FName OverridePrefix)
{
	SlotWorldTransform = GetGripSlotTransform(CallingController);
    SlotName = FName(TEXT("Magazine"));
    bHadSlotInRange = true;
}

bool ATVRMagazine::DenyGripping_Implementation(UGripMotionControllerComponent* GripInitiator)
{
	if(IsInserted())
	{
		if(const auto Parent = GetRootComponent()->GetAttachParent())
		{
			if(const auto Gun = Cast<ATVRGunBase>(Parent->GetOwner()))
			{
				if(const auto GunParent = Gun->GetRootComponent()->GetAttachParent())
				{
					return GunParent->GetClass()->IsChildOf(UTVREquipmentPoint::StaticClass());
				}
			}
		}
	}
	return false;
}

void ATVRMagazine::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController,
	const FBPActorGripInformation& GripInformation)
{
	Super::OnGrip_Implementation(GrippingController, GripInformation);
	bIsMagReleasePressed = false;
}

void ATVRMagazine::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController,
	const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	Super::OnGripRelease_Implementation(ReleasingController, GripInformation, bWasSocketed);
	bIsMagReleasePressed = false;
}

bool ATVRMagazine::SimulateOnDrop_Implementation()
{
	if(IsInserted())
	{
		return false;
	}
	return true;
}

UHandSocketComponent* ATVRMagazine::GetHandSocket_Implementation(FName SlotName) const
{
	return HandSocket;
}


EGripCollisionType ATVRMagazine::GetPrimaryGripType_Implementation(bool bIsSlot)
{
    if(IsInserted())
    {
        return EGripCollisionType::CustomGrip;
    }    
    return EGripCollisionType::InteractiveCollisionWithPhysics;
}

FTransform ATVRMagazine::GetGripSlotTransform_Implementation(UGripMotionControllerComponent* Hand) const
{
	UHandSocketComponent* MyHandSocket = ITVRHandSocketInterface::Execute_GetHandSocket(this, FName(TEXT("Magazine")));
	if(MyHandSocket)
	{
		return MyHandSocket->GetHandSocketTransform(Hand);
	}
    return GripSlot->GetComponentTransform();
}

void ATVRMagazine::InitMagazine()
{
}

void ATVRMagazine::SetCollisionProfile(FName NewProfile)
{
	for(UStaticMeshComponent* LoopMesh : MagazineMeshes)
	{
		if(LoopMesh)
		{
			LoopMesh->SetCollisionProfileName(NewProfile);
		}
	}
}

void ATVRMagazine::OnMagFullyEjected(const FVector& AngularVelocity, const FVector& LinearVelocity)
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	GetStaticMeshComponent()->SetCollisionProfileName(COLLISION_WEAPON);
    GetStaticMeshComponent()->SetSimulatePhysics(true);
    GetStaticMeshComponent()->AddImpulse(LinearVelocity, EName::NAME_None, true);
    GetStaticMeshComponent()->AddAngularImpulseInDegrees(AngularVelocity, EName::NAME_None, true);
	AttachedMagWell = nullptr;

    MagInsertPercentage = 0.f;
    ReInitGrip();
    BP_OnMagFullyEjected();
}

bool ATVRMagazine::TryAttachToWeapon(USceneComponent* AttachComponent, UTVRMagWellComponent* MagWell, const FTransform& AttachTransform, float Progress)
{
    if(!IsInserted())
    {
    	AttachedMagWell = MagWell;
        
        const FAttachmentTransformRules AttachRule(EAttachmentRule::KeepWorld, true);
        GetStaticMeshComponent()->AttachToComponent(AttachComponent, AttachRule);
        
        ReInitGrip();
        SetMagazineOriginToTransform(AttachTransform);
    	MagInsertPercentage = Progress;
    	
        return true;
    }
    return false;
}

bool ATVRMagazine::TryAttachToWeapon(USceneComponent* AttachComponent, UTVRMagazineWell* MagWell,
	const FTransform& AttachTransform, const float Progress)
{
	if(!IsInserted())
	{
		AttachedMagWell = MagWell;
        
		const FAttachmentTransformRules AttachRule(EAttachmentRule::KeepWorld, true);
		GetStaticMeshComponent()->AttachToComponent(AttachComponent, AttachRule);
        
		ReInitGrip();
		SetMagazineOriginToTransform(AttachTransform);
		MagInsertPercentage = Progress;
    	
		return true;
	}
	return false;
}

bool ATVRMagazine::SetMagazineOriginToTransform(const FTransform& NewTransform, bool bSweep, FTransform& outTransform)
{
	const FTransform OriginTransform = AttachOrigin->GetRelativeTransform();
	const FTransform NewTransformWS = OriginTransform.Inverse() * NewTransform;
	if(bSweep && AttachedMagWell)
	{
		TArray<FHitResult> SweepArray;
		FHitResult SweepResult;
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if(const auto Gun = Cast<ATVRGunBase>(AttachedMagWell->GetOwner()))
		{
			QueryParams.AddIgnoredActor(Gun);
			TArray<AActor*> OwnerChildren;
			Gun->GetAllChildActors(OwnerChildren);
			QueryParams.AddIgnoredActors(OwnerChildren);
			if(Gun->GetPrimaryController())
			{				
				if(const auto Hand = UTVRFunctionLibrary::GetGraspingHandForController(Gun->GetPrimaryController()))
				{					
					QueryParams.AddIgnoredActor(Hand);
				}
			}
			if(const auto ParentGun = Gun->GetParentGun())
			{
				QueryParams.AddIgnoredActor(ParentGun);
				Gun->GetAllChildActors(OwnerChildren);
			}
			QueryParams.AddIgnoredActors(OwnerChildren);
		}	
		const FVector StartLoc = DropCollider->GetComponentLocation();
		const FVector EndLoc = NewTransformWS.TransformPosition(DropCollider->GetRelativeLocation());
		const FVector SweepDir = (EndLoc - StartLoc).GetSafeNormal();
		const FVector RelLocWS = NewTransformWS.TransformVector(DropCollider->GetRelativeLocation());

		constexpr float MoveBackDistance = 0.5f;
		
		if(GetWorld()->SweepSingleByChannel(
			SweepResult,
			StartLoc - (SweepDir * MoveBackDistance), EndLoc,
			DropCollider->GetComponentRotation().Quaternion(),
			ECC_WeaponObjectChannel,
			DropCollider->GetCollisionShape(0.25),
			QueryParams))
		 {
			const FVector SweepLoc = SweepResult.Location - RelLocWS;
			const FTransform SweepedTF(NewTransformWS.GetRotation(), SweepLoc);
			
			SetActorTransform(SweepedTF, false);
			outTransform = OriginTransform * SweepedTF;
			return true;
		}
	}
	SetActorTransform(NewTransformWS, false);
	outTransform = NewTransform;
	return false;
}

bool ATVRMagazine::SetMagazineOriginToTransform(const FTransform& NewTransform)
{
	const FTransform OriginTransform = AttachOrigin->GetRelativeTransform();
	const FTransform NewTransformWS = OriginTransform.Inverse() * NewTransform;
	SetActorTransform(NewTransformWS, false);
	return false;
}

bool ATVRMagazine::IsInserted() const
{
    return AttachedMagWell != nullptr;
}

bool ATVRMagazine::IsEmpty() const
{
    return CurrentAmmo <= 0;
}

bool ATVRMagazine::TryConsumeAmmo()
{
    if(IsEmpty())
    {
        return false;
    }
    SetAmmo(CurrentAmmo-1);
    return true;
}

void ATVRMagazine::ReInitGrip()
{
    if(VRGripInterfaceSettings.bIsHeld)
    {
        if(UGripMotionControllerComponent* HoldingHand = VRGripInterfaceSettings.HoldingControllers[0].HoldingController)
        {           
            const FTransform RelTransform = HoldingHand->GrippedObjects[0].RelativeTransform; 
            HoldingHand->DropObjectByInterface(this, 0, FVector::ZeroVector, FVector::ZeroVector);
            HoldingHand->GripObjectByInterface(this,
            	RelTransform, true,
            	EName::NAME_None,
            	FName(TEXT("Magazine")),
            	true);
        }
    }
}

void ATVRMagazine::SetAmmo(int32 NewAmmo)
{
    if(CurrentAmmo != NewAmmo)
    {
        CurrentAmmo = FMath::Clamp(NewAmmo, 0, AmmoCapacity);
    	UpdateRoundInstances();
		UpdateFollowerLocation();

    	BP_OnAmmoChanged();
    }
}

bool ATVRMagazine::IsMagReleasePressed() const
{
	return bIsMagReleasePressed;
}

void ATVRMagazine::OnMagReleasePressed()
{
	if(IsInserted() && !bIsMagReleasePressed && HasMagWellMagRelease())
	{
		ITVRMagazineInterface::Execute_OnMagReleasePressed(AttachedMagWell, true);
		bIsMagReleasePressed = true;
	}
}

void ATVRMagazine::OnMagReleaseReleased()
{
	if(IsInserted() && bIsMagReleasePressed && HasMagWellMagRelease())
	{
		bIsMagReleasePressed = false;
		ITVRMagazineInterface::Execute_OnMagReleaseReleased(AttachedMagWell, true);
	}
}

FTransform ATVRMagazine::GetRoundTransform_Implementation(int32 Index) const
{
	const bool bHasCurve = CurveStartIdx > 0 && CurveStartIdx < AmmoCapacity;
	const int32 IdxStraightPart = bHasCurve ? FMath::Min(Index, CurveStartIdx) : Index;
	float RightCoordStraight, UpCoordStraight;
	if(bDoubleStack)
	{
		const float RLModifier = ((CurrentAmmo % 2) > 0) == ((Index % 2) > 0) != bSwitchLROrder ? -1.f : 1.f;
		RightCoordStraight = -RoundRadius* FMath::Sin(PI/3)*RLModifier;
		UpCoordStraight = -static_cast<float>(IdxStraightPart) * RoundRadius;
	}
	else
	{
		RightCoordStraight = 0.f;
		UpCoordStraight = -static_cast<float>(IdxStraightPart) * 2.f * RoundRadius;
	}
	
	const float ForwardCoordStraight = StackSlope * static_cast<float>(Index);

	if(bHasCurve && CurveRadius > 0.f)
	{
		const int32 IdxCurvedPart = FMath::Max(Index - CurveStartIdx, 0);
		const float RoundAngleRad = (bDoubleStack ? RoundRadius : 2.f * RoundRadius) / CurveRadius * static_cast<float>(IdxCurvedPart);
		const float ForwardCoordCurve = CurveRadius - FMath::Cos(-RoundAngleRad) * CurveRadius;
		const float UpCoordCurve = FMath::Sin(-RoundAngleRad) * CurveRadius;

		const FVector RoundLoc(ForwardCoordCurve + ForwardCoordStraight, RightCoordStraight, UpCoordStraight + UpCoordCurve);
		const FRotator RoundRot(FMath::RadiansToDegrees(RoundAngleRad), 0.f, 0.f);
		return FTransform(RoundRot, RoundLoc, RoundScale);
	}
	else
	{
		const FVector RoundLoc(ForwardCoordStraight, RightCoordStraight, UpCoordStraight);
		return FTransform(FRotator::ZeroRotator, RoundLoc, RoundScale);
	}
}

void ATVRMagazine::UpdateFollowerLocation_Implementation()
{
	if(USkeletalMeshComponent* Spring = GetSpringComponent())
	{
		Spring->SetMorphTarget(FName("Compression"), static_cast<float>(GetDisplayAmmo()) * FollowerMorphSlope + FollowerMorphBias, true);
	}
	if(GetFollowerComponent())
	{
		FVector NewLocation;
		FRotator NewRotation;
		GetFollowerLocationAndRotation(NewLocation, NewRotation);
		GetFollowerComponent()->SetRelativeLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::None);
	}
}

void ATVRMagazine::GetFollowerLocationAndRotation_Implementation(FVector& OutVector, FRotator& OutRotator) const
{
	const FTransform AmmoTransform = GetRoundTransform(GetDisplayAmmo());
	const FRotator TempRot = AmmoTransform.GetRotation().Rotator();
	OutVector = AmmoTransform.GetLocation() * FVector(1.f, 0.f, 1.f) + FollowerOffset;
	const float TempY = OutVector.Y;
	OutVector.Y = OutVector.X;
	OutVector.X = TempY;
	OutRotator = FRotator(-TempRot.Roll, TempRot.Yaw, -TempRot.Pitch);
}

FVector ATVRMagazine::GetCenterOfMass() const
{
	return GetActorTransform().TransformPosition(SavedCenterOfMass);
}


void ATVRMagazine::UpdateRoundInstances_Implementation()
{
	if(UInstancedStaticMeshComponent* Rounds = GetRoundsComponent())
	{
		// adjust instance count
		const int32 InstancesToRemove = Rounds->GetInstanceCount() - GetDisplayAmmo();
		if(InstancesToRemove > 0)
		{
			for(int32 i = 0; i < InstancesToRemove; i++)
			{
				Rounds->RemoveInstance(Rounds->GetInstanceCount() - 1);
			}
		}
		else if(InstancesToRemove < 0)
		{    			
			for(int32 i = 0; i > InstancesToRemove; i--)
			{
				Rounds->AddInstance(FTransform::Identity);
			}
		}

		for(int32 i = 0; i < Rounds->GetInstanceCount(); i++)
		{
			const FTransform NewTransform = GetRoundTransform(i);
			Rounds->UpdateInstanceTransform(i, NewTransform, false, false, true);
		}
		Rounds->MarkRenderStateDirty();
	}
}

int32 ATVRMagazine::GetDisplayAmmo() const
{
	if(LimitDisplayAmmo <= 0)
	{
		return CurrentAmmo;
	}
	return FMath::Min(LimitDisplayAmmo, CurrentAmmo);
}

bool ATVRMagazine::HasMagWellMagRelease() const
{
	if(AttachedMagWell)
	{
		if(const auto MagWell = Cast<UTVRMagWellComponent>(AttachedMagWell))
		{
			return MagWell->HasMagRelease();
		}
		if(const auto MagWell2 = Cast<UTVRMagazineWell>(AttachedMagWell))
		{
			return MagWell2->HasMagRelease();
		}
	}
	return false;
}
