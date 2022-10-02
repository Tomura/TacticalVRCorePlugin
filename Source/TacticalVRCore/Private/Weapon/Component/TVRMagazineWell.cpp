// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Weapon/Component/TVRMagazineWell.h"

#include "TacticalCollisionProfiles.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/NavLinkProxy.h"
#include "Player/TVRCharacter.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/TVRCartridge.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"


#define MAG_AUDIO_StartInsert 0
#define MAG_AUDIO_FullyInserted 1
#define MAG_AUDIO_StartDrop 2
#define MAG_AUDIO_FullyDropped 3


UTVRMagazineWell::UTVRMagazineWell(const FObjectInitializer& OI)
{
	EjectRelativeLoc = FVector(0.f, 0.f, -1.f);
	CollisionExtent = FVector::OneVector;

	CollisionBox = nullptr;
	MagAudioComp = nullptr;

	MagazineSound = nullptr;
	CurrentMagazine = nullptr;
	
	bIsMagFree = false;
	MagVelocity = FVector::ZeroVector;
	
	bMagReleasePressed = false;
	bUseCurve = false;
	bWasReleasedByHand = true;
	bNeedsToBeReleasedByHand = false;
	bHasMagRelease = false;
	DefaultMagazineClass = nullptr;
	MagEjectAcceleration = 0.f;
}


void UTVRMagazineWell::CreateChildActor()
{
	Super::CreateChildActor();

	if(const auto NewChild = GetChildActor())
	{
		if(const auto NewMag = Cast<ATVRMagazine>(NewChild))
		{
			NewMag->SetMagazineOriginToTransform(GetComponentTransform());
		}
	}
	
	InitChildMag();
}

void UTVRMagazineWell::OnRegister()
{
	Super::OnRegister();
}

void UTVRMagazineWell::OnUnregister()
{
	Super::OnUnregister();
}

void UTVRMagazineWell::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UTVRMagazineWell::BeginPlay()
{
	Super::BeginPlay();
	if(MagazineSound && !MagAudioComp)
	{
		MagAudioComp = NewObject<UAudioComponent>(this);
		MagAudioComp->bAutoActivate = false;
		MagAudioComp->SetSound(MagazineSound);
		MagAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	if(!CollisionBox)
	{
		CollisionBox = NewObject<UBoxComponent>(this);
		CollisionBox->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CollisionBox->SetRelativeTransform(CollisionRelativeTransform);
		CollisionBox->SetBoxExtent(CollisionExtent);
		CollisionBox->SetCollisionProfileName(COLLISION_MAGAZINE_INSERT, false);
	}
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &UTVRMagazineWell::OnBeginOverlap);
	
	InitChildMag();
}

void UTVRMagazineWell::BeginDestroy()
{
	Super::BeginDestroy();
	if(CollisionBox && !CollisionBox->IsPendingKill())
	{
		CollisionBox->DestroyComponent();
	}
	if(MagAudioComp && !MagAudioComp->IsPendingKill())
	{
		MagAudioComp->DestroyComponent();
	}
}

ATVRGunBase* UTVRMagazineWell::GetGunOwner() const
{
	if(GetOwner())
	{
		if(const auto Gun = Cast<ATVRGunBase>(GetOwner()))
		{
			return Gun;
		}
		if(const auto Attachment = Cast<ATVRWeaponAttachment>(GetOwner()))
		{
			return Attachment->GetGunOwner();
		}
	}
	return nullptr;
}

void UTVRMagazineWell::InitChildMag()
{
	if(const auto Gun = GetGunOwner())
	{
		if(GetChildActor() && HasBegunPlay())
		{
			if(const auto MyMag = Cast<ATVRMagazine>(GetChildActor()))
			{
				MyMag->TryAttachToWeapon(Gun->GetStaticMeshComponent(), this, GetComponentTransform(), 1.f);
				CurrentMagazine = MyMag;
				OnMagFullyInserted();
			}
		}
	}
}

void UTVRMagazineWell::SetMagazineCollisionProfile_Implementation(FName NewProfile)
{
	ITVRMagazineInterface::SetMagazineCollisionProfile_Implementation(NewProfile);
	if(ATVRMagazine* Mag = GetCurrentMagazine())
	{
		Mag->SetCollisionProfile(NewProfile);
	}
}

bool UTVRMagazineWell::CanBoltLock_Implementation() const
{
	return HasFullyInsertedMagazine() && GetCurrentMagazine()->IsEmpty();
}

bool UTVRMagazineWell::IsEmpty_Implementation() const
{
	if(HasFullyInsertedMagazine())
	{
		return GetCurrentMagazine()->IsEmpty();
	}
	return true; // really? todo: check behavior without mag
}

float UTVRMagazineWell::GetAmmoInsertProgress_Implementation() const
{
	if(const ATVRMagazine* const Mag = GetCurrentMagazine())
	{
		return Mag->MagInsertPercentage;
	}
	return  0.f;
}

void UTVRMagazineWell::OnMagReleasePressed_Implementation(bool bAlternatePress)
{
	const bool bAlreadyPressed = Execute_IsMagReleasePressed(this);
	if(!bAlternatePress && !bMagReleasePressed)
	{
		bMagReleasePressed = true;
	}
	if(!bAlreadyPressed)
	{
		TryReleaseMag();
		if(EventOnMagReleasePressed.IsBound())
		{
			EventOnMagReleasePressed.Broadcast();
		}
	}
}

void UTVRMagazineWell::OnMagReleaseReleased_Implementation(bool bAlternatePress)
{
	if(!bAlternatePress && bMagReleasePressed)
	{
		bMagReleasePressed = false;
	}
	const bool bStillPressed = Execute_IsMagReleasePressed(this);
	if(!bStillPressed)
	{
		if(EventOnMagReleaseReleased.IsBound())
		{
			EventOnMagReleaseReleased.Broadcast();
		}
	}
}

void UTVRMagazineWell::OnOwnerGripReleased_Implementation(ATVRCharacter* OwningChar,
	UGripMotionControllerComponent* ReleasingHand)
{
	if(GetCurrentMagazine() == nullptr)
	{
		return;
	}
	
	const auto OtherHand = OwningChar->GetOtherControllerHand(ReleasingHand);
	if(OtherHand)
	{
		FBPActorGripInformation GripInfo;
		EBPVRResultSwitch Result;
		OtherHand->GetGripByActor(GripInfo, GetCurrentMagazine(), Result);
		if(Result == EBPVRResultSwitch::OnSucceeded)
		{			
			OtherHand->DropObjectByInterface(GetCurrentMagazine());
		}
	}
}

bool UTVRMagazineWell::CanFeedAmmo_Implementation() const
{
	// As for now two conditions
	// 1) There needs to be a magazine
	// 2) The magazine needs to be fully attached
    
	return HasFullyInsertedMagazine() && !GetCurrentMagazine()->IsEmpty();
}

TSubclassOf<ATVRCartridge> UTVRMagazineWell::TryFeedAmmo_Implementation()
{
	if(Execute_CanFeedAmmo(this) && GetCurrentMagazine()->TryConsumeAmmo())
	{
		return GetCurrentMagazine()->GetCartridgeType();
	}
	return nullptr;
}

void UTVRMagazineWell::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(const auto Gun = GetGunOwner())
	{
		if(CanInsertMag())
		{
			ATVRMagazine* Mag = Cast<ATVRMagazine>(OtherActor);
			if(Mag != nullptr && IsAllowedMagType(Mag->GetClass()) && Mag->VRGripInterfaceSettings.bIsHeld)
			{
				StartInsertMagazine(Mag);
			}
		}
	}
}


void UTVRMagazineWell::HandleMagDrop(float DeltaSeconds)
{
	if(const auto Gun = GetGunOwner())
	{    
		if(HasMagazine() && bIsMagFree)
		{
			const bool bEjectMag = ShouldEjectMag();
			const bool bInsertMag = ShouldInsertMag();
	        
			if(!bEjectMag && !bInsertMag)
			{
				if(GetCurrentMagazine()->VRGripInterfaceSettings.bIsHeld)
				{
					HandleMagInsert(DeltaSeconds);
				}
				else if(!bNeedsToBeReleasedByHand || bWasReleasedByHand)
				{
					HandleMagFall(DeltaSeconds);
				}
			}
			else if(bEjectMag)
			{
				OnMagFullyEjected();
			}
			else
			{
				OnMagFullyInserted();
			}
		}
	}
}

void UTVRMagazineWell::HandleMagInsert(float DeltaSeconds)
{
	ATVRGunBase* Gun = GetGunOwner();
	
	bWasReleasedByHand = true;
	MagVelocity = FVector::ZeroVector;
	UGripMotionControllerComponent* GrippingHand = CurrentMagazine->VRGripInterfaceSettings.HoldingControllers[0].HoldingController;               
	const FTransform HandTransform = GrippingHand->GetComponentTransform();
	const FTransform MagSlot = CurrentMagazine->GetGripSlotTransform(GrippingHand);
	const FTransform MagOrigin = CurrentMagazine->GetAttachOrigin()->GetComponentTransform();
	
	FTransform QueryTransform = MagOrigin * MagSlot.Inverse() * HandTransform;
	
	const float Progress = GetSplineTransform(QueryTransform.GetLocation(), QueryTransform);
	
	MagVelocity = (QueryTransform.GetLocation() - MagOrigin.GetLocation())/DeltaSeconds;
	CurrentMagazine->SetMagazineOriginToTransform(QueryTransform);
	CurrentMagazine->MagInsertPercentage = Progress;
}

void UTVRMagazineWell::HandleMagFall(float DeltaSeconds)
{
	const ATVRGunBase* Gun = GetGunOwner();
	const auto MagCoM = CurrentMagazine->GetCenterOfMass();	
	const auto RotVel = Gun->GetStaticMeshComponent()->GetPhysicsAngularVelocityInRadians();
	const auto CentrifugalAcc = RotVel ^ (RotVel ^ (Gun->GetActorLocation()-MagCoM));
	
	const float DropAcceleration = GetWorld()->GetGravityZ();
	MagVelocity = MagVelocity + (FVector::UpVector * DropAcceleration + CentrifugalAcc) * DeltaSeconds;
	MagVelocity -= GetUpVector() * MagEjectAcceleration * DeltaSeconds;
	const FVector MagLoc = GetCurrentMagazine()->GetAttachOrigin()->GetComponentLocation();
	const FVector ExternalVelocity = Gun->GetStaticMeshComponent()->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
	const FVector DesiredMagLocInternal = MagLoc + (MagVelocity) * DeltaSeconds;
	const FVector DesiredMagLoc = DesiredMagLocInternal - ExternalVelocity * DeltaSeconds;
	
	FTransform FoundTransform;
	GetSplineTransform(DesiredMagLoc, FoundTransform);
	CurrentMagazine->SetMagazineOriginToTransform(FoundTransform, true, FoundTransform);
	// refine transform again
	const float Progress = GetSplineTransform(FoundTransform.GetLocation(), FoundTransform);
	MagVelocity = (FoundTransform.GetLocation() - MagLoc)/DeltaSeconds;
	
	CurrentMagazine->MagInsertPercentage = Progress;

}

void UTVRMagazineWell::OnMagFullyEjected()
{
	if(HasMagazine())
	{
		bWasReleasedByHand = false;
		CurrentMagazine->SetMagazineOriginToTransform(GetComponentTransform());
		CurrentMagazine->MagInsertPercentage = 1.f;
		bIsMagFree = false;

		if(MagAudioComp)
		{
			MagAudioComp->Stop();
			MagAudioComp->SetIntParameter(FName("MagEvent"), MAG_AUDIO_FullyInserted);
			MagAudioComp->Play();
		}
    	
		if(EventOnMagazineFullyInserted.IsBound())
		{
			EventOnMagazineFullyInserted.Broadcast();
		}
	}
}

void UTVRMagazineWell::OnMagDestroyed()
{
	bIsMagFree= false;
	CurrentMagazine = nullptr;
}

void UTVRMagazineWell::OnMagFullyInserted()
{
	if(HasMagazine())
	{
		bWasReleasedByHand = false;
		CurrentMagazine->SetMagazineOriginToTransform(GetComponentTransform());
		CurrentMagazine->MagInsertPercentage = 1.f;
		bIsMagFree = false;

		if(MagAudioComp)
		{
			MagAudioComp->Stop();
			MagAudioComp->SetIntParameter(FName("MagEvent"), MAG_AUDIO_FullyInserted);
			MagAudioComp->Play();
		}
    	
		if(EventOnMagazineFullyInserted.IsBound())
		{
			EventOnMagazineFullyInserted.Broadcast();
		}
	}
}

bool UTVRMagazineWell::ShouldEjectMag() const
{
	if(HasMagazine())
	{
		const float Progress = GetCurrentMagazine()->GetInsertProgress();
		return Progress <= 0.f; 
	}
	return true;
}

bool UTVRMagazineWell::ShouldInsertMag() const
{
	if(Execute_IsMagReleasePressed(this))
	{
		return false;
	}
    
	if(HasMagazine())
	{
		const float Progress = GetCurrentMagazine()->GetInsertProgress();
		return Progress >= 0.99f; 
	}
	return true;
}

bool UTVRMagazineWell::CanInsertMag() const
{
	if (HasMagazine()) {
		return false;
	}

	const auto Gun = GetGunOwner();

	if (Gun->VRGripInterfaceSettings.bIsHeld)
	{
		return true;
	}

	if (AActor* GunParent = Gun->GetAttachParentActor()) // for cornershot
		{
		return Cast<ATVRGunBase>(GunParent) != nullptr;
		}
	return false;
}


bool UTVRMagazineWell::TryReleaseMag()
{
	if(HasMagazine() && !bIsMagFree)
	{
		MagVelocity = FVector::ZeroVector;
		bIsMagFree = true;

		if(MagAudioComp)
		{
			MagAudioComp->Stop();
			MagAudioComp->SetIntParameter(FName("MagEvent"), MAG_AUDIO_StartDrop);
			MagAudioComp->Play();
		}
    	
		if(EventOnMagazineStartDrop.IsBound())
		{
			EventOnMagazineStartDrop.Broadcast();
		}
		return true;
	}
	return false;
}

bool UTVRMagazineWell::IsMagReleasePressed_Implementation() const
{
	return bMagReleasePressed || (GetCurrentMagazine() && GetCurrentMagazine()->IsMagReleasePressed());
}

void UTVRMagazineWell::GetAllowedCatridges_Implementation(TArray<TSubclassOf<ATVRCartridge>>& OutCartridges) const
{
	for(const auto MagClass : AllowedMagazines)
	{
		const auto MagCDO = GetDefault<ATVRMagazine>(MagClass);
		OutCartridges.AddUnique(MagCDO->CartridgeType);
	}
}

void UTVRMagazineWell::StartInsertMagazine(ATVRMagazine* MagToInsert)
{
	const auto Gun = GetGunOwner();
	if(Gun == nullptr)
	{
		return;
	}
	
	// const FTransform SplineTransform = GetMagSpline()->GetTransformAtTime(0.f, ESplineCoordinateSpace::World, false);
	FTransform FoundTransform;
	const float NewProgress = GetSplineTransform(
		MagToInsert->GetAttachOrigin()->GetComponentLocation(), FoundTransform);
	if(MagToInsert->TryAttachToWeapon(Gun->GetStaticMeshComponent(), this, FoundTransform, NewProgress))
	{
		bIsMagFree = true;
		CurrentMagazine = MagToInsert;

		if(MagAudioComp)
		{
			MagAudioComp->Stop();
			MagAudioComp->SetIntParameter(FName("MagEvent"), MAG_AUDIO_StartInsert);
			MagAudioComp->Play();
		}
		if(EventOnMagazineStartInsert.IsBound())
		{
			EventOnMagazineStartInsert.Broadcast();
		}
	}
}

float UTVRMagazineWell::GetSplineTransform(const FVector& inLoc, FTransform& outTransform) const
{
	const FVector EjectLocWS = GetEjectLocation();
	const FVector StartLocWS = GetComponentLocation();
	
	FVector ClosestPoint = UKismetMathLibrary::FindClosestPointOnSegment(
		inLoc, StartLocWS, EjectLocWS);
	
	const float EjectLengthSq = (EjectLocWS - StartLocWS).SizeSquared();	
	const float Progress = ((ClosestPoint - StartLocWS) | (EjectLocWS - StartLocWS)) / EjectLengthSq;
	FRotator Rot = GetComponentRotation();
	if(bUseCurve)
	{	
		const float Roll = MagRoll.GetRichCurveConst()->Eval(Progress);
		const float Pitch = MagPitch.GetRichCurveConst()->Eval(Progress);
		const float Yaw = MagYaw.GetRichCurveConst()->Eval(Progress);
		Rot = FRotator(Pitch, Yaw, Roll);
	}
	outTransform = FTransform(Rot, ClosestPoint);
	return FMath::Clamp(Progress, 0.f, 1.f);
}


void UTVRMagazineWell::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    HandleMagDrop(DeltaTime);
}
