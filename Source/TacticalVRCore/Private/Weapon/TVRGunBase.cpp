// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRGunBase.h"

#include "TacticalCollisionProfiles.h"
#include "Components/ArrowComponent.h"
#include "Weapon/TVRMagazine.h"
#include "Player/TVRCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Libraries/TVRFunctionLibrary.h"

#include "Player/TVREquipmentPoint.h"
#include "Settings/TVRCoreGameplaySettings.h"

#include "Weapon/Component/TVRMagWellComponent.h"
#include "Weapon/Component/TVRMagazineCompInterface.h"
#include "Weapon/Component/TVRTriggerComponent.h"

#include "Weapon/TVRGunWithChild.h"

#include "Weapon/TVRGunAnimInstance.h"
#include "Weapon/TVRCartridge.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "Weapon/Attachments/WPNA_Barrel.h"
#include "Weapon/Attachments/WPNA_ForeGrip.h"
#include "Weapon/Attachments/WPNA_Laser.h"
#include "Weapon/Attachments/WPNA_Light.h"
#include "Weapon/Attachments/WPNA_Sight.h"
#include "Weapon/Attachments/WPNA_PistolLight.h"
#include "Weapon/Attachments/WPNA_UnderbarrelWeapon.h"
#include "Weapon/Component/TVRAttachPoint_Barrel.h"
#include "Weapon/Component/TVRChargingHandleInterface.h"
#include "Weapon/Component/TVREjectionPort.h"
#include "Weapon/Component/TVRGunFireComponent.h"

FName ATVRGunBase::PrimarySlotName(TEXT("Primary"));
FName ATVRGunBase::SecondarySlotName(TEXT("Secondary"));

ATVRGunBase::ATVRGunBase(const FObjectInitializer& OI) : Super(OI)
{
    GripScript = CreateDefaultSubobject<UGS_GunTools>(FName("GSGunTools"));
    GripLogicScripts.Add(GripScript);

    VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_SlotOnly;
    VRGripInterfaceSettings.MovementReplicationType = EGripMovementReplicationSettings::ClientSide_Authoritive;
    VRGripInterfaceSettings.ConstraintStiffness = 5000.f;
    VRGripInterfaceSettings.ConstraintDamping = 300.f;
    VRGripInterfaceSettings.ConstraintBreakDistance = 100.f;
    VRGripInterfaceSettings.AdvancedGripSettings.GripPriority = 1;
    VRGripInterfaceSettings.AdvancedGripSettings.PhysicsSettings.bUsePhysicsSettings = true;
    VRGripInterfaceSettings.AdvancedGripSettings.PhysicsSettings.bTurnOffGravityDuringGrip = true;
	VRGripInterfaceSettings.SecondarySlotRange = 10.f;
    
    bReplicates = true;
    bAlwaysRelevant = true;
    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GripType.Large")));

    GetStaticMeshComponent()->SetCollisionProfileName(COLLISION_WEAPON);
	GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
	GetStaticMeshComponent()->SetNotifyRigidBodyCollision(true); // hit events

    bIsSocketed = false;
    SavedSecondaryHand = nullptr;
	
	TriggerComponent = CreateDefaultSubobject<UTVRTriggerComponent>(FName(TEXT("Trigger")));
	
	FiringComponent = CreateDefaultSubobject<UTVRGunFireComponent>(FName(TEXT("FiringComponent")));
	FiringComponent->SetupAttachment(GetStaticMeshComponent());
	
	GunManipulationAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName(TEXT("GunManipulationAudio")));	
	GunManipulationAudioComponent->SetupAttachment(GetStaticMeshComponent());
	GunManipulationAudioComponent->SetAutoActivate(false);

	MovablePartsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("Movables")));
	MovablePartsMesh->SetupAttachment(GetStaticMeshComponent());
	MovablePartsMesh->SetCollisionProfileName(COLLISION_NO_COLLISION);
	
    bBoltReleasePressed = false;
	
    bIsBoltLocked = false;
	
    bHandSwapToPrimaryGripSlot = false;
	HandSwapType = ETVRHandSwapType::KeepWorldPosition;
    bHasLastRoundBoltHoldOpen = true;
	bDoesCycle = true;

    bSkipHandSwap = false;

    PrimaryActorTick.bCanEverTick = true;
	
	GunType = ETVRGunType::Primary;

	SetTickGroup(TG_PostPhysics);

	RecoilImpulse = FVector(20000.f, 1000.f, 0.f);
	// RecoilForceTwoHand = FVector(10000.f, 500.f, 0.f);
	RecoilReductionTwoHand = 0.5f;
	
	OneHandStiffness = 2000.f;
	OneHandDamping = 150.f;
	OneHandAngularStiffness = OneHandStiffness * 0.5f;
	OneHandAngularDamping = OneHandDamping * 1.4f;
	
	TwoHandStiffness = 5000.f;
	TwoHandDamping = 300.f;
	TwoHandAngularStiffness = TwoHandStiffness * 1.5f;
	TwoHandAngularDamping = TwoHandDamping * 1.4f;
	
	bHasMagReleaseOnPrimaryGrip = true;
	bHasBoltReleaseNearPrimaryGrip = true;
	
	SecondaryController = nullptr;

	MagInterface = nullptr;

	BoltProgress = 0.f;
	BoltMovePct = -1.f;

	BoltProgressOpenDustCover = 0.1f;
	BoltProgressEjectRound = 0.97f;
	BoltProgressFeedRound = 0.8f;
	BoltStiffness = 100.f;
	BoltProgressSpeed = 0.f;
	BoltStroke = 10.f;
	BoltProgressHammerCocked = 0.5f;

	ChargingHandleInterface = nullptr;
	BoltMesh = nullptr;
	SelectorSound = nullptr;
	SelectorAudio = nullptr;

	bForceRecompile = false;
}

void ATVRGunBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	OnColorVariantChanged(ColorVariant);
	InitAttachmentPoints();

	if(bForceRecompile)
	{
		bForceRecompile = false;
	}
}

void ATVRGunBase::BeginPlay()
{
    Super::BeginPlay();

	OnColorVariantChanged(ColorVariant);
	InitAttachmentPoints();
	
	InitChargingHandle();

	if(GetFiringComponent())
	{
		GetFiringComponent()->OnFire.AddDynamic(this, &ATVRGunBase::OnFire);
		GetFiringComponent()->OnCartridgeSpent.AddDynamic(this, &ATVRGunBase::OnCartridgeSpent);
		GetFiringComponent()->OnEmpty.AddDynamic(this, &ATVRGunBase::OnEmpty);
		GetFiringComponent()->OnEndCycle.AddDynamic(this, &ATVRGunBase::OnEndFiringCycle);
	}	

	if(TriggerComponent)
	{
		TriggerComponent->OnTriggerActivate.AddDynamic(this, &ATVRGunBase::OnStartFire);
		TriggerComponent->OnTriggerReset.AddDynamic(this, &ATVRGunBase::OnStopFire);
	}
	
    for(UActorComponent* TestMesh : GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("LoadedBullet")))
    {
        LoadedBullet = Cast<UStaticMeshComponent>(TestMesh);
    	if(LoadedBullet != nullptr)
    	{
    		LoadedBullet->SetVisibility(GetFiringComponent()->HasRoundLoaded());
    	}
    }
	
	CollectWeaponMeshes();

	InitMagInterface();
	InitEjectionPort();

	OnActorHit.AddDynamic(this, &ATVRGunBase::OnPhysicsHit);

	if(SelectorSound)
	{
		SelectorAudio = NewObject<UAudioComponent>(this);
		SelectorAudio->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		SelectorAudio->SetAutoActivate(false);
		SelectorAudio->SetSound(SelectorSound);
	}
}

void ATVRGunBase::InitAttachmentPoints()
{
    GetComponents<UTVRAttachmentPoint>(AttachmentPoints);
	for(UTVRAttachmentPoint* AttachPoint: AttachmentPoints)
	{
		AttachPoint->OnConstruction();
	}
	
	const auto Sight = GetAttachment<AWPNA_Sight>();
	if(Sight)
	{
		FoldSights(Sight->bFoldIronSights);
		HideRearSight(Sight->bHideRearSight);
		if(const auto Barrel = GetAttachment<AWPNA_Barrel>())
		{
			Barrel->OnFoldSights(Sight->bFoldIronSights);
		}
	}
	else
	{
		FoldSights(false);
		HideRearSight(false);
		if(const auto Barrel = GetAttachment<AWPNA_Barrel>())
		{
			Barrel->OnFoldSights(false);
		}
	}
}

void ATVRGunBase::Tick(float DeltaSeconds)
{	
    Super::Tick(DeltaSeconds);
	TickBolt(DeltaSeconds);
	TickHammer(DeltaSeconds);
}

void ATVRGunBase::BeginDestroy()
{
    if(GripScript != nullptr)
    {
        GripLogicScripts.Remove(GripScript);
    }
	
    Super::BeginDestroy();
}

void ATVRGunBase::TickBolt(float DeltaSeconds)
{	
	const float PreviousBoltProgress = BoltProgress;
	if(bDoesCycle && FiringComponent->IsInFiringCooldown()) 
	{
		// there is one problem, with fire rates that would finish in less than one frame,
		// but that's an unrealistic use-case. Generally weapons should have fire rates that occupy more than 3 frames
		// worst case frame rate it seen as 36 FPS (oculus quest in re-projection)
		
		const float PrevBoltMovePct = BoltMovePct;
		BoltMovePct = 2.f * (1.f - FiringComponent->GetRefireCooldownRemainingPct()) - 1.f; // -1: 0s, 0: on max deflection, 1: end
		const float FirePct = 1.f - FMath::Abs(BoltMovePct);
		BoltProgress = FirePct;

		// we only check MovePct in this mode, this gives us info about the entire firing process
		// from moving the bolt to resetting it properly, where as with bolt process we do not know
		// in which stage we are.
		// Because of this it is safe to change bolt progress for visual purposes.
		
		if(PrevBoltMovePct <= (BoltProgressEjectRound - 1.f) && BoltMovePct > (BoltProgressEjectRound - 1.f))
		{
			EjectRound();
			UnlockBoltIfNecessary();
		}
		if(PrevBoltMovePct <= (1.f - BoltProgressEjectRound) && BoltMovePct > (1.f - BoltProgressEjectRound))
		{
			LockBoltIfNecessary();
		}

		if(IsBoltLocked()) // && bIsResetting && BoltProgress < BoltProgressEjectRound)
		{            
			BoltProgress = BoltProgressEjectRound;
		}
		
		if(PrevBoltMovePct <= (1.f - BoltProgressFeedRound) && BoltMovePct > (1.f - BoltProgressFeedRound))
		{
			TryFeedRoundFromMagazine();
		}

		if(BoltProgress <= 0.f) // unlikely to happen, but just to make sure, we reset value on bolt closure
		{
			BoltProgress = 0.f;
			BoltProgressSpeed = 0.f;
			BoltMovePct = -1.f;
		}
	}
	else // usually here we are utilising the charging handle or the bolt is resetting from being released
	{
		if(!IsBoltLocked())
		{
			if(BoltProgress > 0.f && (!GetChargingHandleInterface() || !ITVRChargingHandleInterface::Execute_IsInUse(GetChargingHandleInterface())))
			{
				// BoltProgress = FMath::FInterpConstantTo(BoltProgress, 0.f, DeltaSeconds, BoltStiffness);
				// const float BoltDeflection = BoltProgress * BoltStroke;
				const float Acceleration = BoltStiffness * BoltProgress;
				BoltProgressSpeed -= Acceleration * DeltaSeconds;
				BoltProgress = BoltProgress + BoltProgressSpeed * DeltaSeconds;
				if(BoltProgress <= 0.f)
				{
					OnBoltClosed();
				}
			}
			else
			{
				BoltProgress = 0.f;
			}
		}
		else // bolt is locked
		{
			BoltProgressSpeed = 0.f;
			BoltProgress = BoltProgressEjectRound;
		}
		
		if(GetChargingHandleInterface())
		{
			const float ChargingHandleProgress = ITVRChargingHandleInterface::Execute_GetProgress(GetChargingHandleInterface());
			BoltProgress = FMath::Max(ChargingHandleProgress, BoltProgress);
			if(ITVRChargingHandleInterface::Execute_IsInUse(GetChargingHandleInterface()))
			{
				BoltProgressSpeed = 0.f;
			}
		}
		CheckBoltEvents(PreviousBoltProgress);
	}

	if(BoltMesh && BoltProgress != PreviousBoltProgress)
	{
		BoltMesh->SetRelativeLocation(BoltMeshInitialRelativeLocation - FVector(0.f, BoltProgress*BoltStroke, 0.f));
	}

	if(BoltProgress > BoltProgressOpenDustCover)
	{
		OnOpenDustCover();
	}
	

}

void ATVRGunBase::TickHammer(float DeltaSeconds)
{
	if(!bHammerLocked)
	{
		const float TriggerProgress = GetTriggerComponent()->GetTriggerValue();
		const float TriggerActivate = GetTriggerComponent()->GetTriggerActivateValue();
		const float TriggerReset = GetTriggerComponent()->GetTriggerResetValue();
		
		const float HammerTrigger = GetTriggerComponent()->DoesTriggerNeedReset() || (!bHammerDoubleAction) ? 0.f :
			FMath::Clamp((TriggerProgress-TriggerReset)/(TriggerActivate-TriggerReset), 0.f, 1.f);
		const float HammerBolt = FMath::Clamp(BoltProgress / BoltProgressHammerCocked, 0.f, 1.f);
		HammerProgress = FMath::Max(HammerBolt, HammerTrigger);
		if(HammerProgress >= 1.f)
		{
			bHammerLocked = true;
		}
	}
}

void ATVRGunBase::CheckBoltEvents(float PreviousBoltProgress)
{
	if(PreviousBoltProgress <= BoltProgressEjectRound && BoltProgress > BoltProgressEjectRound)
	{
		EjectRound();
		UnlockBoltIfNecessary();
	}
		
	if(PreviousBoltProgress >= BoltProgressEjectRound && BoltProgress < BoltProgressEjectRound)
	{
		LockBoltIfNecessary();
	}
		
	if(PreviousBoltProgress >= BoltProgressFeedRound && BoltProgress < BoltProgressFeedRound)
	{
		TryFeedRoundFromMagazine();
	}
}

void ATVRGunBase::OnEndFiringCycle()
{
	const float PreviousBoltProgress = BoltProgress;
	BoltProgress = 0;
	CheckBoltEvents(PreviousBoltProgress);
}

void ATVRGunBase::ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot,
                                                        bool& bHadSlotInRange, FTransform& SlotWorldTransform, FName& SlotName,
                                                        UGripMotionControllerComponent* CallingController, FName OverridePrefix)
{
    if(!bSecondarySlot)
    {
    	UHandSocketComponent* PrimarySocket = GetPrimaryHandSocket();

    	const FTransform SlotTransform = PrimarySocket ?
    		GetPrimaryHandSocket()->GetHandSocketTransform(CallingController) :
    		FTransform::Identity;
        const float RangeSquared = VRGripInterfaceSettings.PrimarySlotRange*VRGripInterfaceSettings.PrimarySlotRange;
        if((SlotTransform.GetLocation() - WorldLocation).SizeSquared() <= RangeSquared)
        {
            bHadSlotInRange = true;
            SlotName = ATVRGunBase::PrimarySlotName;
            SlotWorldTransform = SlotTransform;
            return;
        }
    }
    else
    {
	    if(GetSecondarySlot(WorldLocation, SlotWorldTransform, SlotName, CallingController))
	    {
	    	bHadSlotInRange = true;
	    	const UTVRCoreGameplaySettings* TVRGameplaySettingsCDO = GetDefault<UTVRCoreGameplaySettings>();
	    	switch (TVRGameplaySettingsCDO->GunStockType)
	    	{
	    	case EStockType::ST_None:
			case EStockType::ST_VirtualStock:
	    		break;
	    	case EStockType::ST_PhysicalStock:
	    		// in case of a physical stock, we want consistency, so that one stock setting works for all guns.
	    		// we assume that both controllers are in one line.
	    		// a future implementation could capture the controller positions in the stock and use that capture to get a suitable point
	    		// from the stocks settings.
	            			
	    		const FVector SecondaryGripOffset = TVRGameplaySettingsCDO->PhysicalStockSecondaryOffset;
	    		const FVector SecondaryLoc = GetPrimaryGripSlot()->GetComponentLocation()
	    			+ GetActorRightVector() * SecondaryGripOffset.X
					- GetActorForwardVector() * SecondaryGripOffset.Y
	    			+ GetActorUpVector() * SecondaryGripOffset.Z;
	    		const FRotator SecondaryRot = UKismetMathLibrary::MakeRotFromXZ(GetActorRightVector(), GetActorUpVector());
	    		SlotWorldTransform = FTransform(SecondaryRot, SecondaryLoc, FVector::OneVector);
	    	}
	    	return;
	    }
    }
    bHadSlotInRange = false;
}

bool ATVRGunBase::GetSecondarySlot(FVector WorldLocation, FTransform& OutTransform, FName& OutSlotName,
	UGripMotionControllerComponent* CallingController) const
{
	if(const auto Foregrip = GetAttachment<AWPNA_ForeGrip>())
	{
		if(Foregrip->GetGripSlot(WorldLocation, CallingController, OutTransform, OutSlotName))
		{						
			return true; 
		}
	}
	if(const auto Barrel = GetAttachment<AWPNA_Barrel>())
	{
		if(Barrel->GetGripSlot(WorldLocation, CallingController, OutTransform, OutSlotName))
		{					
			return true;
		}
	}
	if(GetSecondarySlotComponent() != nullptr)
	{
		const float RangeSquared = VRGripInterfaceSettings.SecondarySlotRange * VRGripInterfaceSettings.SecondarySlotRange;
		USplineComponent* SecondarySpline = Cast<USplineComponent>(GetSecondarySlotComponent());
		const FTransform BestTransform = SecondarySpline ?
				SecondarySpline->FindTransformClosestToWorldLocation(
					WorldLocation, ESplineCoordinateSpace::World, false) :
				GetSecondarySlotComponent()->GetComponentTransform();
		
		if((BestTransform.GetLocation() - WorldLocation).SizeSquared() <= RangeSquared)
		{
			OutTransform = BestTransform;
			OutSlotName = SecondarySlotName;
			return true;
		}
	}
	return false;
}

bool ATVRGunBase::RequestsSocketing_Implementation(USceneComponent*& ParentToSocketTo, FName& OptionalSocketName,
                                                   FTransform_NetQuantize& RelativeTransform)
{
	// primary was released, lets check whether there is a secondary:
	if(const auto CharacterOwner = Cast<ATVRCharacter>(GetOwner()))
	{
		TArray<UTVREquipmentPoint*> Slots;
		CharacterOwner->GetComponents<UTVREquipmentPoint>(Slots);
		const FVector HandSocketLoc = GetPrimaryHandSocket()->GetComponentLocation();
		Slots.Sort([HandSocketLoc] (const UTVREquipmentPoint& A, const UTVREquipmentPoint& B)
		{
			return (A.GetComponentLocation() - HandSocketLoc).SizeSquared() > (B.GetComponentLocation() - HandSocketLoc).SizeSquared();
		});

		for(const auto Slot: Slots)
		{
			if(Slot->CanAcceptGun(this))
			{				
				ParentToSocketTo = Slot;
				RelativeTransform =GetTransform().GetRelativeTransform(ParentToSocketTo->GetComponentTransform());
				return true;
			}
		}
	}
	return false;
}

bool ATVRGunBase::DenyGripping_Implementation(UGripMotionControllerComponent* GripInitiator)
{
	return IsHeldByParentGun();
}

UHandSocketComponent* ATVRGunBase::GetHandSocket_Implementation(FName SocketName) const
{
	if(SocketName == PrimarySlotName)
	{
		return GetPrimaryHandSocket();
	}
	
	if(SocketName == SecondarySlotName)
	{
		return GetSecondaryHandSocket();
	}
	
	if(const auto ForeGrip = GetAttachment<AWPNA_ForeGrip>())
	{		
		return Execute_GetHandSocket(ForeGrip, SocketName);
	}
	
	if(const auto BarrelAttachment = GetAttachment<AWPNA_Barrel>())
	{		
		return Execute_GetHandSocket(BarrelAttachment, SocketName);
	}
	return nullptr;
}

ACharacter* ATVRGunBase::GetCharacterOwner() const
{
	if(GetOwner())
	{
		return Cast<ACharacter>(GetOwner());
	}
	return nullptr;
}

ATVRCharacter* ATVRGunBase::GetVRCharacterOwner() const
{
	if(GetOwner())
	{
		return Cast<ATVRCharacter>(GetOwner());
	}
	return nullptr;
}

AController* ATVRGunBase::GetOwnerController() const
{
	if(const auto Char = GetCharacterOwner())
	{
		return Char->GetController();
	}
	return nullptr;
}

bool ATVRGunBase::IsOwnerLocalPlayerController() const
{
	if(const auto OwnerController = GetOwnerController())
	{
		return OwnerController->IsLocalPlayerController();
	}
   return false;
}

bool ATVRGunBase::IsHeldByParentGun() const
{
	return GetParentGun() != nullptr;
}

ATVRGunWithChild* ATVRGunBase::GetParentGun() const
{
	if(AActor* Parent = GetRootComponent()->GetAttachmentRootActor())
	{
		if(ATVRGunWithChild* ParentGun = Cast<ATVRGunWithChild>(Parent))
		{
			if(ParentGun != this)
			{
				return ParentGun;
			}
		}	
	}
	return nullptr;
}

void ATVRGunBase::SetCollisionProfile_Implementation(FName NewProfile)
{
	for(UStaticMeshComponent* LoopMesh: GunMeshes)
	{
		if(LoopMesh != nullptr)
		{
			LoopMesh->SetCollisionProfileName(NewProfile);
		}
	}

	if(GetMagInterface())
	{
		GetMagInterface()->SetMagazineCollisionProfile(NewProfile);
	}

	for(UTVRAttachmentPoint* TestPoint : AttachmentPoints)
	{
		if(TestPoint)
		{
			if(ATVRWeaponAttachment* LoopAttachment = TestPoint->GetCurrentAttachment())
			{
				LoopAttachment->SetCollisionProfile(NewProfile);
			}
		}
	}
}

void ATVRGunBase::OnFullyDropped(ATVRCharacter* GrippingCharacter, UGripMotionControllerComponent* GrippingHand)
{
	SetOwner(nullptr);
	if(GetMagInterface())
	{
		GetMagInterface()->OnOwnerGripReleased(GrippingCharacter, GrippingHand);
	}
	BPOnFullyDropped();
}

void ATVRGunBase::SetConstraintToTwoHanded()
{
	if(VRGripInterfaceSettings.HoldingControllers.IsValidIndex(0)) // is there even a grip?
	{
		UGripMotionControllerComponent* PrimaryGripController = VRGripInterfaceSettings.HoldingControllers[0].HoldingController;
		FBPActorGripInformation GripInfo;
		EBPVRResultSwitch Result;
		PrimaryGripController->GetGripByID(GripInfo, VRGripInterfaceSettings.HoldingControllers[0].GripID, Result);
		if(Result == EBPVRResultSwitch::OnSucceeded)
		{
			PrimaryGripController->SetGripStiffnessAndDamping(
				GripInfo, Result,
				TwoHandStiffness, TwoHandDamping,
				true,
				TwoHandAngularStiffness, TwoHandAngularDamping
			);
		}
	}
}

void ATVRGunBase::SetConstraintToOneHanded()
{
	if(VRGripInterfaceSettings.HoldingControllers.IsValidIndex(0)) // is there even a grip?
	{
		UGripMotionControllerComponent* PrimaryGripController = VRGripInterfaceSettings.HoldingControllers[0].HoldingController;
		FBPActorGripInformation GripInfo;
		EBPVRResultSwitch Result;
		PrimaryGripController->GetGripByID(GripInfo, VRGripInterfaceSettings.HoldingControllers[0].GripID, Result);
		if(Result == EBPVRResultSwitch::OnSucceeded)
		{
			PrimaryGripController->SetGripStiffnessAndDamping(
				GripInfo, Result,
				OneHandStiffness, OneHandDamping,
				true,
				OneHandAngularStiffness, OneHandAngularDamping
			);
		}
	}
}

void ATVRGunBase::OnPhysicsHit(AActor* HitActor, AActor* OtherActor, FVector HitVelocity, const FHitResult& Hit)
{
}

void ATVRGunBase::SetColorVariant(uint8 newVariant)
{
	ColorVariant = newVariant;
	OnColorVariantChanged(ColorVariant);
}

void ATVRGunBase::SetBoltMesh(UStaticMeshComponent* NewMesh)
{
	BoltMesh = NewMesh;
	if(BoltMesh)
	{		
		BoltMeshInitialRelativeLocation = BoltMesh->GetRelativeLocation();
	}
}

void ATVRGunBase::OnGrip_Implementation(UGripMotionControllerComponent* GrippingHand,
                                        const FBPActorGripInformation& GripInfo)
{
    Super::OnGrip_Implementation(GrippingHand, GripInfo);
	
    if(bIsSocketed)
    {
        FBPActorGripInformation Grip = GripInfo;
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);        
        GrippingHand->TeleportMoveGrip(Grip, true, true);
    }
    bIsSocketed = false;
    if(GripInfo.bIsSlotGrip)
    {
        // todo: deny free gripping
        const ATVRGunBase* DefaultCDO = GetDefault<ATVRGunBase>(GetClass());
        VRGripInterfaceSettings.SecondaryGripType = DefaultCDO->VRGripInterfaceSettings.SecondaryGripType;
    	PrimaryHand = GrippingHand;
    	TriggerComponent->ActivateTrigger(GrippingHand);
    }
    else
    {
        VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_None;
    }
    ReInitSecondary(GrippingHand, GripInfo);

	AddTickPrerequisiteComponent(GrippingHand);
	AddTickPrerequisiteActor(GrippingHand->GetOwner());
	ATVRCharacter* GrippingChar = Cast<ATVRCharacter>(GrippingHand->GetOwner());
	if(GrippingChar)
	{
		AddTickPrerequisiteComponent(GrippingChar->VRMovementReference);
	}	
	
	if(EventOnGripped.IsBound())
	{
		EventOnGripped.Broadcast(GrippingHand, GripInfo);
	}

	SetConstraintToOneHanded();
	FTimerHandle T;
	GetWorldTimerManager().SetTimer(T, this, &ATVRGunBase::SetConstraintToOneHanded, 0.5f);
}

void ATVRGunBase::OnGripRelease_Implementation(UGripMotionControllerComponent* GrippingHand,
    const FBPActorGripInformation& GripInfo, bool bWasSocketed)
{
    bIsSocketed = bWasSocketed;

	RemoveTickPrerequisiteComponent(GrippingHand);
	RemoveTickPrerequisiteActor(GrippingHand->GetOwner());	
	ATVRCharacter* GrippingChar = Cast<ATVRCharacter>(GrippingHand->GetOwner());
	if(GrippingChar)
	{
		RemoveTickPrerequisiteComponent(GrippingChar->VRMovementReference);
	}
    
    if(GripInfo.SlotName == PrimarySlotName)
    {
        OnMagReleaseReleased();
        OnBoltReleaseReleased();
        OnStopFire();
    	TriggerComponent->DeactivateTrigger();
    	PrimaryHand = nullptr;
    }
    
    Super::OnGripRelease_Implementation(GrippingHand, GripInfo, bWasSocketed);
    // todo: Allow Free Gripping
	bool bPerformedHandSwap = false;
    if(!bSkipHandSwap)
    {
        bPerformedHandSwap = HandleHandSwap(GrippingHand, GripInfo);
    }

	if(GetOwner())
    {
	    const auto CharOwner = Cast<ATVRCharacter>(GetOwner());
		FBPActorGripInformation Grip;
		EBPVRResultSwitch Result;
		EControllerHand Hand;
		GrippingHand->GetHandType(Hand);
		CharOwner->GetOtherControllerHand(Hand)->GetGripByActor(Grip, this, Result);
		if(Result == EBPVRResultSwitch::OnFailed)
		{			
			OnFullyDropped(CharOwner, GrippingHand);
		}
    }
	
	if(!bPerformedHandSwap)
	{
	}

	if(EventOnDropped.IsBound())
	{
		EventOnDropped.Broadcast(GrippingHand, GripInfo, bWasSocketed);
	}
}

void ATVRGunBase::OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController,
	USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation)
{
	Super::OnSecondaryGrip_Implementation(GripOwningController, SecondaryGripComponent, GripInformation);
	SecondaryController = Cast<UGripMotionControllerComponent>(GripInformation.SecondaryGripInfo.SecondaryAttachment);
	SecondaryGripInfo = GripInformation;

	if(GripInformation.SecondaryGripInfo.bIsSlotGrip && GripInformation.SecondaryGripInfo.SecondarySlotName != SecondarySlotName)
	{
		if(const auto ForeGrip = GetAttachment<AWPNA_ForeGrip>())
		{
			ForeGrip->OnGripped(SecondaryController, SecondaryGripInfo, true);
		}
	}

	if(EventOnSecondaryGripped.IsBound())
	{
		EventOnSecondaryGripped.Broadcast(GripOwningController, GripInformation);
	}
	
	SetConstraintToTwoHanded();
}

void ATVRGunBase::OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController,
	USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation)
{
	Execute_OnEndSecondaryUsed(this);
	Super::OnSecondaryGripRelease_Implementation(GripOwningController, ReleasingSecondaryGripComponent, GripInformation);
	if(const auto ForeGrip = GetAttachment<AWPNA_ForeGrip>())
	{
		ForeGrip->OnReleased(SecondaryController, SecondaryGripInfo, true);		
	}
	SecondaryController = nullptr;

	if(EventOnSecondaryDropped.IsBound())
	{
		EventOnSecondaryDropped.Broadcast(GripOwningController, GripInformation);
	}
	
	SetConstraintToOneHanded();
}

void ATVRGunBase::OnSecondaryUsed_Implementation()
{
	Super::OnSecondaryUsed_Implementation();
	if(EventOnSecondaryUsed.IsBound())
	{
		EventOnSecondaryUsed.Broadcast();
	}
}

void ATVRGunBase::OnEndSecondaryUsed_Implementation()
{
	Super::OnEndSecondaryUsed_Implementation();
	if(EventOnSecondaryEndUsed.IsBound())
	{
		EventOnSecondaryEndUsed.Broadcast();
	}
}

void ATVRGunBase::ReInitSecondary(UGripMotionControllerComponent* GrippingHand,
                                  const FBPActorGripInformation& GripInfo)
{
    if(GrippingHand->HasGripAuthority(GripInfo) && SavedSecondaryHand != GrippingHand)
    {
        if(SavedSecondaryHand != nullptr)
        {
            UGripMotionControllerComponent* OldSecondaryHand = SavedSecondaryHand;
            const FTransform OldSecondaryTransform = SavedSecondaryHandRelTransform;
        	if(const auto GraspingHand = UTVRFunctionLibrary::GetGraspingHandForController(SavedSecondaryHand))
        	{
        		GraspingHand->bPendingReinitSecondary = true;
        	}
            SavedSecondaryHand->DropObject(this, 0, false, FVector::ZeroVector, FVector::ZeroVector);
            GrippingHand->AddSecondaryAttachmentToGrip(GripInfo, OldSecondaryHand, OldSecondaryTransform, true, 0.f, true, SavedSecondarySlotName);
        }
        SavedSecondaryHand = nullptr;
        VRGripInterfaceSettings.bAllowMultipleGrips = false;
    }
}

bool ATVRGunBase::HandleHandSwap(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo)
{
	// todo: handle hand swap when hand is on magazine and magazine is attached
	// todo: handle hand swap when magazine is grabbed and dropping
    if(GripInfo.SecondaryGripInfo.bHasSecondaryAttachment)
    {
        const auto SecondaryHand = Cast<UGripMotionControllerComponent>(GripInfo.SecondaryGripInfo.SecondaryAttachment);
        if(SecondaryHand != nullptr && SecondaryHand->HasGripAuthority(this))
        {
            SavedSecondaryHand = SecondaryHand;
            SavedSecondaryHandRelTransform = GripInfo.SecondaryGripInfo.SecondaryRelativeTransform;
        	SavedSecondarySlotName = GripInfo.SecondaryGripInfo.bIsSlotGrip ? GripInfo.SecondaryGripInfo.SecondarySlotName : NAME_None;

        	switch(HandSwapType)
        	{
        	case ETVRHandSwapType::KeepWorldPosition:
        		{
        			bool bHadSecondarySlotInRange = false;
        			FTransform SecondarySlotTransform;
        			FName TempSecondaryName = EName::NAME_None;
        	
        			Execute_ClosestGripSlotInRange(this, SavedSecondaryHand->GetComponentLocation(), true,
						bHadSecondarySlotInRange, SecondarySlotTransform, TempSecondaryName,
						SavedSecondaryHand, EName::NAME_None
					);
        		
        			if(bHadSecondarySlotInRange)
        			{
        				const FTransform RelativeSocketTransform =
        					SavedSecondaryHand->ConvertToControllerRelativeTransform(GetActorTransform());
        					// GetActorTransform().GetRelativeTransform(SavedSecondaryHand->GetComponentTransform());
        				if(const auto GraspingHand = UTVRFunctionLibrary::GetGraspingHandForController(SavedSecondaryHand))
        				{
        					GraspingHand->PendingHandSwap = ETVRHandSwapType::KeepWorldPosition;
        					const auto HandTF = GraspingHand->GetSkeletalMeshComponent()->GetComponentTransform();
        					GraspingHand->PendingRelativeMeshTransform = HandTF * GetActorTransform().Inverse();
        					GraspingHand->SnapShotCustomPose();
        					GraspingHand->FreezePose();
						}
        				SavedSecondaryHand->GripObjectByInterface(this, RelativeSocketTransform,
							true, EName::NAME_None,
							SavedSecondarySlotName, true);
        				VRGripInterfaceSettings.bAllowMultipleGrips = true;
        				return true;
        			}
        		}
        		break;
        	case ETVRHandSwapType::GripPrimary:
        		{
        			if(const auto MyChar = Cast<ATVRCharacter>(SavedSecondaryHand->GetOwner()))
        			{
        				bool bHadSlotInRange = false;
        				FName SlotName = EName::NAME_None;        	
        				FTransform SlotTransform;
        	
        				Execute_ClosestGripSlotInRange(this, GrippingHand->GetComponentLocation(), false,
							bHadSlotInRange, SlotTransform, SlotName,
							SavedSecondaryHand, EName::NAME_None
						);
                	
        				// FTransform SocketTransformWS = GetPrimaryHandSocket()->GetHandSocketTransform(SavedSecondaryHand.Get());
        				const FTransform RelativeSocketTransform = GetActorTransform().GetRelativeTransform(SlotTransform);
        				SavedSecondaryHand->GripObjectByInterface(
							this, RelativeSocketTransform, true,
							EName::NAME_None,
							PrimarySlotName,
							true);
        				return true;
        			}
        		}
        		break;
        	}
        }
    }
    SavedSecondaryHand = nullptr;
	return false;
}


USceneComponent* ATVRGunBase::GetSecondarySlotComponent_Implementation() const
{
	if(const auto HandSocket = GetSecondaryHandSocket())
	{
		return HandSocket;
	}
	return nullptr;
}

void ATVRGunBase::OnStartFire()
{
    if(CanStartFire())
    {
    	GetFiringComponent()->StartFire();
    }
}

void ATVRGunBase::OnStopFire()
{
	FiringComponent->StopFire();
}



bool ATVRGunBase::HasLastRoundBoltHoldOpen() const
{
    return bHasLastRoundBoltHoldOpen;
}


void ATVRGunBase::AddRecoil()
{
	FTransform RecoilPOA;
	GetRecoilPointOfAttack(RecoilPOA);
	FVector RecoilImpulseToApply = RecoilImpulse;
	FVector AngularRecoilImpulseToApply = RecoilAngularImpulse;

	for(const auto AttachPoint: AttachmentPoints)
	{
		if(const auto LoopWPNA = AttachPoint->GetCurrentAttachment())
		{
			RecoilImpulseToApply *= LoopWPNA->GetRecoilModifier();
			AngularRecoilImpulseToApply *= LoopWPNA->GetRecoilModifier();
		}
	}
	
    if(VRGripInterfaceSettings.bIsHeld)
    {
    	AddCustomRecoil(RecoilPOA, RecoilImpulseToApply, AngularRecoilImpulseToApply);
    }
	else if(IsHeldByParentGun())
	{
		ATVRGunWithChild* ParentGun = Cast<ATVRGunWithChild>(GetStaticMeshComponent()->GetAttachmentRootActor());
		ParentGun->AddCustomRecoil(RecoilPOA, RecoilImpulseToApply, AngularRecoilImpulseToApply);
	}
}

void ATVRGunBase::AddCustomRecoil(const FTransform& PointOfAttack, const FVector& Impulse,
	const FVector& AngularImpulse)
{
	float RecoilModifier = 1.f;
	if(VRGripInterfaceSettings.bIsHeld)
	{
		UGripMotionControllerComponent* Hand = VRGripInterfaceSettings.HoldingControllers[0].HoldingController;
		FBPActorGripInformation Grip;
		EBPVRResultSwitch Result;
        Hand->GetGripByID(Grip, VRGripInterfaceSettings.HoldingControllers[0].GripID, Result);
		if(Result == EBPVRResultSwitch::OnSucceeded)
		{
			if(Grip.SecondaryGripInfo.bHasSecondaryAttachment)
			{
				RecoilModifier *= RecoilReductionTwoHand;
			}
		}
	}
    const float RecoilRandomRightScale = RecoilStream.FRandRange(-1.f, 1.f);

	const FVector RelativeRecoilImpulse = RecoilModifier * Impulse * FVector(-1.f, RecoilRandomRightScale, 1.f);
	const FVector RelativeAngularImpulse = RecoilModifier* AngularImpulse;
	
	GetStaticMeshComponent()->GetBodyInstance()->AddImpulseAtPosition(
		PointOfAttack.TransformVectorNoScale(RelativeRecoilImpulse),
		PointOfAttack.GetLocation());
	GetStaticMeshComponent()->GetBodyInstance()->AddAngularImpulseInRadians(
		PointOfAttack.TransformVectorNoScale(RelativeAngularImpulse),
		true);
}

void ATVRGunBase::OnFire()
{
	AddRecoil();
	// we need to restart the cycling procedure, if we do not initialize it with -1
	// some variables will not correspond to their actual meaning during the first tick
	// after firing.
	BoltMovePct = -1.f;
	bHammerLocked = false;
	HammerProgress = 0.f;
}

void ATVRGunBase::OnEmpty()
{
	bHammerLocked = false;
	HammerProgress = 0.f;
}

void ATVRGunBase::OnCartridgeSpent()
{
	if(FiringComponent->GetLoadedCartridge() == nullptr)
		return;
	if(LoadedBullet == nullptr)
		return;
	
	const auto CartridgeCDO = FiringComponent->GetLoadedCartridge()->GetDefaultObject<ATVRCartridge>();
	if(UStaticMesh* SpentMesh = CartridgeCDO->GetSpentCartridgeMesh())
	{			
		LoadedBullet->SetStaticMesh(SpentMesh);
	}
}


bool ATVRGunBase::CanStartFire() const
{
    if(FiringComponent->IsInFiringCooldown())
    {
        return false;
    }
	if(GetChargingHandleInterface() && ITVRChargingHandleInterface::Execute_IsInUse(GetChargingHandleInterface()))
	{
		return false;
	}
	// if(BoltProgress > SMALL_NUMBER)
	// {
	// 	return false;
	// }
    return IsGrippedAtPrimaryGrip() || IsHeldByParentGun();
}

USceneComponent* ATVRGunBase::GetChargingHandleInterface() const
{
	return ChargingHandleInterface;
}

void ATVRGunBase::OnBoltReleased()
{
    UnlockBolt();
    BP_OnBoltReleased();
}

void ATVRGunBase::OnMagReleasePressedFromPrimary()
{
	if(CanUseMagRelease() && HasMagReleaseOnPrimaryGrip())
	{
		OnMagReleasePressed();
	}
}

void ATVRGunBase::OnMagReleaseReleasedFromPrimary()
{
	if(HasMagReleaseOnPrimaryGrip())
	{
		OnMagReleaseReleased();
	}
}

void ATVRGunBase::OnMagReleasePressed()
{
	if(GetMagInterface())
	{
		GetMagInterface()->OnMagReleasePressed();
	}
	BP_OnMagReleasePressed();
}

void ATVRGunBase::OnMagReleaseReleased()
{
    if(GetMagInterface())
    {
        GetMagInterface()->OnMagReleaseReleased();
    }
    BP_OnMagReleaseReleased();
}

bool ATVRGunBase::OnSecondaryMagReleasePressed()
{
	if(const auto UnderBarrelWeapon = GetAttachment<AWPNA_UnderbarrelWeapon>())
	{
		return UnderBarrelWeapon->OnMagReleasePressed();
	}
	return false;
}

bool ATVRGunBase::OnSecondaryMagReleaseReleased()
{
	if(const auto UnderBarrelWeapon = GetAttachment<AWPNA_UnderbarrelWeapon>())
	{
		return UnderBarrelWeapon->OnMagReleaseReleased();
	}
	return false;
}

bool ATVRGunBase::OnSecondaryBoltReleasePressed()
{
	if(const auto UnderBarrelWeapon = GetAttachment<AWPNA_UnderbarrelWeapon>())
	{
		return UnderBarrelWeapon->OnBoltReleasePressed();
	}
	return false;
}

bool ATVRGunBase::OnSecondaryBoltReleaseReleased()
{
	if(const auto UnderBarrelWeapon = GetAttachment<AWPNA_UnderbarrelWeapon>())
	{
		return UnderBarrelWeapon->OnBoltReleaseReleased();
	}
	return false;
}


void ATVRGunBase::InitMagInterface_Implementation()
{
	const auto FoundComp = GetComponentByClass(UTVRMagazineCompInterface::StaticClass());
	if(FoundComp)
	{
		MagInterface = Cast<UTVRMagazineCompInterface>(FoundComp);
	}
}

void ATVRGunBase::InitEjectionPort()
{
	const auto FoundComp = GetComponentByClass(UTVREjectionPort::StaticClass());
	if(FoundComp)
	{
		EjectionPort = Cast<UTVREjectionPort>(FoundComp);
	}
}

void ATVRGunBase::InitChargingHandle()
{
	TArray<USceneComponent*> Comps;
	GetComponents<USceneComponent>(Comps);
	for(USceneComponent* C : Comps)
	{
		if(C->Implements<UTVRChargingHandleInterface>())
		{
			ChargingHandleInterface = C;
			ITVRChargingHandleInterface::Execute_SetMaxTravel(C, BoltStroke);
			ITVRChargingHandleInterface::Execute_SetStiffness(C, BoltStiffness); 
			break;
		}
	}
}


void ATVRGunBase::EjectRound(bool bSpent)
{
    if(const auto EjectedCartridge = FiringComponent->TryEjectCartridge())
    {
    	const bool CartridgeSpent = FiringComponent->IsCartridgeSpent();
        OnEjectRound(CartridgeSpent, EjectedCartridge);
        if(LoadedBullet)
        {
            LoadedBullet->SetVisibility(false);
        }

    	if(EjectionPort)
    	{
    		EjectionPort->SpawnEjectedCartridge(EjectedCartridge, CartridgeSpent);
    	}
    }
}

bool ATVRGunBase::TryFeedRoundFromMagazine()
{
	if(!GetFiringComponent()->HasRoundLoaded() && GetMagInterface())
	{
		if(const auto NewCartridge = GetMagInterface()->TryFeedAmmo())
		{
			return TryChamberNewRound(NewCartridge);
		}
	}
	return false;
}

bool ATVRGunBase::TryChamberNewRound(TSubclassOf<ATVRCartridge> NewCartridge)
{
    if(FiringComponent->TryLoadCartridge(NewCartridge))
    {
    	if(LoadedBullet)
    	{
    		auto const CartridgeCDO = NewCartridge->GetDefaultObject<ATVRCartridge>();
    		UStaticMesh* RoundMesh = CartridgeCDO->GetStaticMeshComponent()->GetStaticMesh();
    		if(RoundMesh != LoadedBullet->GetStaticMesh())
    		{
    			LoadedBullet->SetStaticMesh(RoundMesh);
    		}
    		LoadedBullet->SetVisibility(true);
    	}
    	OnChamberRound();
    	return true;
    }
    return false;
}

bool ATVRGunBase::CanUseBoltRelease() const
{
    if((VRGripInterfaceSettings.bIsHeld && IsGrippedAtPrimaryGrip()) || IsHeldByParentGun())
    {
        return true;
    }
    return false;
}

bool ATVRGunBase::CanUseMagRelease() const
{
    return IsGrippedAtPrimaryGrip() || IsHeldByParentGun();
}

void ATVRGunBase::OnBoltReleasePressedFromPrimary()
{
	if(CanUseBoltRelease() && HasBoltReleaseOnPrimaryGrip())
	{
		OnBoltReleasePressed();
	}
}

void ATVRGunBase::OnBoltReleaseReleasedFromPrimary()
{
	if(CanUseBoltRelease() && HasBoltReleaseOnPrimaryGrip())
	{
		OnBoltReleaseReleased();
	}
}

void ATVRGunBase::OnBoltReleasePressed()
{
    bBoltReleasePressed = true;
    if(IsBoltLocked())
    {
        OnBoltReleased();
    }
    BP_OnBoltReleasePressed();
}

void ATVRGunBase::OnBoltReleaseReleased()
{
    bBoltReleasePressed = false;
    BP_OnBoltReleaseReleased();
}

void ATVRGunBase::OnCycleFiringMode()
{
    GetFiringComponent()->CycleFireMode();
	if(SelectorAudio)
	{
		SelectorAudio->Stop();
		SelectorAudio->Play();
	}
}


bool ATVRGunBase::IsGrippedAtPrimaryGrip() const
{
    if(VRGripInterfaceSettings.bIsHeld)
    {
        UGripMotionControllerComponent* HoldingHand = VRGripInterfaceSettings.HoldingControllers[0].HoldingController;
        if(HoldingHand)
        {
            FBPActorGripInformation GripInfo;
            EBPVRResultSwitch ResSwitch;
            HoldingHand->GetGripByID(GripInfo, VRGripInterfaceSettings.HoldingControllers[0].GripID, ResSwitch);
            if(ResSwitch == EBPVRResultSwitch::OnSucceeded)
            {
                return GripInfo.SlotName == ATVRGunBase::PrimarySlotName;
            }
        }
    }
    return false;
}


bool ATVRGunBase::IsMagReleasePressed() const
{
    if(GetMagInterface())
    {
        return GetMagInterface()->IsMagReleasePressed();
    }
    return false;
}

void ATVRGunBase::ToggleLaser_Implementation(UGripMotionControllerComponent* UsingHand)
{
	if(const auto Laser = GetAttachment<AWPNA_Laser>())
	{
		Laser->ToggleLaser(UsingHand);
	}
	else if(const auto LaserLight = GetAttachment<AWPNA_PistolLight>())
	{
		LaserLight->ToggleLaser(UsingHand);
	}
}

void ATVRGunBase::ToggleLight_Implementation(UGripMotionControllerComponent* UsingHand)
{
	if(const auto Light = GetAttachment<AWPNA_Light>())
	{
		Light->ToggleLight(UsingHand);
	}
	else if(const auto LaserLight = GetAttachment<AWPNA_PistolLight>())
	{
		LaserLight->ToggleLight(UsingHand);
	}
}

void ATVRGunBase::GetRecoilPointOfAttack_Implementation(FTransform& OutTransform) const
{
	OutTransform = GetFiringComponent()->GetComponentTransform();
}

UGripMotionControllerComponent* ATVRGunBase::GetSecondaryController() const
{
	return SecondaryController;
}

const FBPActorGripInformation* ATVRGunBase::GetSecondaryGripInfo() const
{
	if(GetSecondaryController())
	{
		return &SecondaryGripInfo;
	}
	return nullptr;
}

void ATVRGunBase::OnOpenDustCover()
{
	if(GetMovablePartsMesh())
	{
		if(const auto AnimInstance = GetMovablePartsMesh()->GetAnimInstance())
		{
			if(const auto GunAnimInstance = Cast<UTVRGunAnimInstance>(AnimInstance))
			{
				GunAnimInstance->bOpenDustCover = true;
			}
		}
	}
}

void ATVRGunBase::OnBoltClosed()
{	
	BoltProgress = 0.f;
	if(GetChargingHandleInterface())
	{
		ITVRChargingHandleInterface::Execute_OnBoltClosed(GetChargingHandleInterface());
	}
	if(!IsNetMode(ENetMode::NM_DedicatedServer))
	{
		// const float BoltSpeed = BoltProgressSpeed * BoltStroke;
		OnSimulateBoltClosed();
	}
	BoltProgressSpeed = 0.f;
	BoltMovePct = -1.f;
	if(BoltMesh)
	{
		BoltMesh->SetRelativeLocation(BoltMeshInitialRelativeLocation - FVector(0.f, BoltProgress*BoltStroke, 0.f));
	}
}

void ATVRGunBase::LockBoltIfNecessary()
{
	const bool bShouldLastRoundBHO = HasLastRoundBoltHoldOpen() && GetMagInterface() && GetMagInterface()->CanBoltLock() &&  !GetFiringComponent()->HasRoundLoaded();
	if((bShouldLastRoundBHO || ShouldLockBolt()) && !IsBoltReleasePressed())
	{
		LockBolt();
	}
}

void ATVRGunBase::UnlockBoltIfNecessary()
{
	const bool bUndoBHO = HasLastRoundBoltHoldOpen() && (!GetMagInterface() || !GetMagInterface()->CanBoltLock() || GetFiringComponent()->HasRoundLoaded());
	const bool bUnlockBoltOverride = !HasLastRoundBoltHoldOpen() && !ShouldLockBolt();
	if(IsBoltLocked() && (bUndoBHO || bUnlockBoltOverride))
	{
		UnlockBolt();
	}
}

void ATVRGunBase::LockBolt()
{
	if(!bIsBoltLocked)
	{
		FiringComponent->StopFire();
		bIsBoltLocked = true;
		BoltProgress = BoltProgressEjectRound;
		if(GetChargingHandleInterface())
		{
			ITVRChargingHandleInterface::Execute_LockChargingHandle(GetChargingHandleInterface(), BoltProgress);
		}
	}
}

void ATVRGunBase::UnlockBolt()
{
	if(bIsBoltLocked)
	{		
		bIsBoltLocked = false;
		BoltMovePct = -1.f;
		if(GetChargingHandleInterface())
		{
			ITVRChargingHandleInterface::Execute_UnlockChargingHandle(GetChargingHandleInterface());
		}
	}
}

void ATVRGunBase::CollectWeaponMeshes()
{
	GunMeshes.Empty();
	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);
	for(UStaticMeshComponent* TestMesh : Meshes)
	{
		if(TestMesh->GetCollisionProfileName() == FName("Weapon"))
		{
			GunMeshes.AddUnique(TestMesh);
		}
	}
}
