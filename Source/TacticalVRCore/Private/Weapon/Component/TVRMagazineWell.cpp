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
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
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
	MagazineClass = nullptr;
	MagEjectAcceleration = 0.f;

	bCanDetach = false;
	DetachProgress = 0.f;
}


void UTVRMagazineWell::CreateChildActor(TFunction<void(AActor*)> CustomizerFunc)
{
	Super::CreateChildActor(CustomizerFunc);

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
		MagAudioComp = NewObject<UAudioComponent>(this, FName(TEXT("MagazineAudio")));
		MagAudioComp->RegisterComponent();
		MagAudioComp->bAutoActivate = false;
		MagAudioComp->SetSound(MagazineSound);
		MagAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	if(!CollisionBox)
	{
		CollisionBox = NewObject<UBoxComponent>(this, FName(TEXT("CollisionBox")));
		CollisionBox->RegisterComponent();
		CollisionBox->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CollisionBox->SetRelativeTransform(CollisionRelativeTransform);
		CollisionBox->SetBoxExtent(CollisionExtent);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetCollisionProfileName(COLLISION_MAGAZINE_INSERT, false);
		CollisionBox->SetGenerateOverlapEvents(true);
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &UTVRMagazineWell::OnBeginOverlap);
	}
	
	SpawnMagazineAttached(MagazineClass);
}

ATVRMagazine* UTVRMagazineWell::SpawnMagazineAttached(TSubclassOf<ATVRMagazine> NewMagazineClass)
{
	const auto MagClass = GetChildActorClass();
	if(GetChildActor())
	{
		DestroyChildActor();
	}

	ATVRMagazine* MyMag = nullptr;
	const auto& TempChildren = GetAttachChildren();
	for(const auto TestChild : TempChildren)
	{
		if(const auto TestMag = Cast<ATVRMagazine>(TestChild->GetOwner()))
		{
			TestMag->Destroy();
		}
	}
	
	if(const auto World = GetWorld())
	{
		if(const auto Gun = GetGunOwner())
		{
			if(NewMagazineClass == nullptr)
			{
				if(AllowedMagazines.Num() > 0)
				{
					NewMagazineClass = AllowedMagazines[0];
				}
				else
				{
					return nullptr;
				}
			}
			if(IsAllowedMagType(NewMagazineClass) && GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.bAllowDuringConstructionScript = false;
				// in theory we could allow spawn during construction script, but it would require addtional code like child actor components
				
				auto NewMag = World->SpawnActor<ATVRMagazine>(NewMagazineClass, SpawnParams);
				if(NewMag)
				{
					NewMag->TryAttachToWeapon(Gun->GetStaticMeshComponent(), this, GetComponentTransform(), 1.f);
					CurrentMagazine = NewMag;
					OnMagFullyInserted();
					return NewMag;
				}
			}
		}
	}
	return nullptr;
}

bool UTVRMagazineWell::SetMagazineClass(TSubclassOf<ATVRMagazine> NewClass)
{
	if(NewClass == nullptr)
	{
		MagazineClass = nullptr;
		// OnConstruction();
		return true;
	}	

	if(IsAllowedMagType(NewClass))
	{
		MagazineClass = NewClass;
		// OnConstruction();
		return true;
	}
	return false;
}

void UTVRMagazineWell::OnConstruction()
{
	if(!HasBegunPlay())
	{
		if(MagazineClass && MagazineClass != GetChildActorClass())
		{
			SetChildActorClass(MagazineClass);
		}
		else if (MagazineClass == nullptr) // firsts condition fails on null
		{
			SetChildActorClass(nullptr);
		}
	}
	
	if(!IsTemplate() && CurrentMagazine) // this can create a double execution of setPreferredVariant, but it should be ok
	{
		// CurrentMagazine->SetColorVariant(GetRequestedColorVariant());
	}
}

void UTVRMagazineWell::BeginDestroy()
{
	Super::BeginDestroy();
	if(IsValid(CollisionBox))
	{
		CollisionBox->DestroyComponent();
	}
	if(IsValid(MagAudioComp))
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
		return Mag->GetInsertProgress();
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
		if(const auto Mag = Cast<ATVRMagazine>(OtherActor))
		{
			if(CanInsertMag() &&
				IsAllowedMagType(Mag->GetClass()) &&
				Mag->VRGripInterfaceSettings.bIsHeld &&
				Mag != IgnoreMagazine
			)
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

			if(CurrentMagazine->GetInsertProgress() > 0.f && DetachProgress == 0.f)
			{
				DetachProgress = 0.f;
			}
	        
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
	
	const float Progress = GetConstrainedTransform(QueryTransform.GetLocation(), QueryTransform);
	
	MagVelocity = (QueryTransform.GetLocation() - MagOrigin.GetLocation())/DeltaSeconds;
	CurrentMagazine->SetMagazineOriginToTransform(QueryTransform);
	CurrentMagazine->MagInsertPercentage = Progress;
}

void UTVRMagazineWell::HandleMagFall(float DeltaSeconds)
{
	constexpr float ExternalVelStrength = 0.5f;
	const ATVRGunBase* Gun = GetGunOwner();
	const FVector EjectDir = (GetEjectLocation() - GetComponentLocation()).GetSafeNormal();
	
	const auto MagCoM = CurrentMagazine->GetCenterOfMass();	
	const auto RotVel = Gun->GetStaticMeshComponent()->GetPhysicsAngularVelocityInRadians();
	const auto CentrifugalAcc = RotVel ^ (RotVel ^ (Gun->GetActorLocation()-MagCoM));
	
	const float DropAcceleration = GetWorld()->GetGravityZ();
	const FVector Acceleration = EjectDir * ((FVector::UpVector * DropAcceleration + CentrifugalAcc) | EjectDir);
	MagVelocity = MagVelocity + Acceleration * DeltaSeconds;
	MagVelocity -= GetUpVector() * MagEjectAcceleration * DeltaSeconds;
	MagVelocity = (MagVelocity | EjectDir) * EjectDir;
	
	const FVector MagLoc = GetCurrentMagazine()->GetAttachOrigin()->GetComponentLocation();
	const FVector ExternalVelocity = Gun->GetStaticMeshComponent()->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
	const FVector ConstrainedExtVel = (ExternalVelStrength * ExternalVelocity | EjectDir) * EjectDir;
	const FVector DesiredMagLocInternal = MagLoc + (MagVelocity) * DeltaSeconds;
	const FVector DesiredMagLoc = DesiredMagLocInternal - ConstrainedExtVel * DeltaSeconds;

	FTransform FoundTransform;
	float Progress = GetConstrainedTransform(DesiredMagLoc, FoundTransform);
	
	if(CurrentMagazine->SetMagazineOriginToTransform(FoundTransform, true, FoundTransform))
	{
		Progress = GetConstrainedTransform(FoundTransform.GetLocation(), FoundTransform);
	}
	CurrentMagazine->MagInsertPercentage = Progress;
	if(Progress < 1.f)
	{		
		MagVelocity = (FoundTransform.GetLocation() - MagLoc)/DeltaSeconds + ConstrainedExtVel;
	}
	else
	{
		MagVelocity = FVector::ZeroVector;
	}
}

void UTVRMagazineWell::OnMagFullyEjected()
{
	const ATVRGunBase* Gun = GetGunOwner();    
	if(HasMagazine() && Gun)
	{
		ATVRMagazine* OldMag = CurrentMagazine;    
		bIsMagFree= false;
		CurrentMagazine = nullptr;
        
		const FVector EjectVelocity = MagVelocity;
		const FVector PhysicsVelocity = Gun->GetStaticMeshComponent()->GetPhysicsLinearVelocity();
		const FVector MagFallDir = (GetEjectLocation() - GetComponentLocation()).GetSafeNormal();
		
		const FVector A = PhysicsVelocity - (PhysicsVelocity | MagFallDir) * MagFallDir;
		const FVector MagLinearVelocity = A + EjectVelocity;
		const FVector MagAngularVelocity = Gun->GetStaticMeshComponent()->GetPhysicsAngularVelocityInDegrees();
    	
		if(EventOnMagazineFullyDropped.IsBound())
		{
			EventOnMagazineFullyDropped.Broadcast();
		}
		OldMag->OnMagFullyEjected(MagAngularVelocity, MagLinearVelocity);

		IgnoreMagazine = OldMag;
		GetWorld()->GetTimerManager().SetTimer(IgnoreMagTimer, this, &UTVRMagazineWell::StopIgnoreMag, 1.f, false);
	}
	else
	{
		// in this case we only modify the variables
		bIsMagFree= false;
		CurrentMagazine = nullptr;
	}
}

void UTVRMagazineWell::StopIgnoreMag()
{
	IgnoreMagazine = nullptr;
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
		return Progress <= DetachProgress;
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

void UTVRMagazineWell::GetAllowedMagazines(TArray<TSubclassOf<ATVRMagazine>>& OutMagazines)
{
	OutMagazines.Append(AllowedMagazines);
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
	const float NewProgress = GetConstrainedTransform(
		MagToInsert->GetAttachOrigin()->GetComponentLocation(), FoundTransform);
	UE_LOG(LogTemp, Log, TEXT("Starting Insert at Progress: %f"), NewProgress);
	DetachProgress = FMath::Min(0.f, NewProgress - 0.01f);
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

float UTVRMagazineWell::GetConstrainedTransform(const FVector& inLoc, FTransform& outTransform) const
{
	const FVector EjectLocWS = GetEjectLocation();
	const FVector StartLocWS = GetComponentLocation();
	
	FVector ClosestPoint = UKismetMathLibrary::FindClosestPointOnSegment(
		inLoc, StartLocWS, EjectLocWS + EjectLocWS - StartLocWS);
	
	const float EjectLengthSq = (EjectLocWS - StartLocWS).SizeSquared();	
	const float Progress = ((ClosestPoint - EjectLocWS) | (StartLocWS - EjectLocWS)) / EjectLengthSq;
	FQuat Rot = GetComponentRotation().Quaternion();
	if(bUseCurve)
	{	
		const float Roll = MagRoll.GetRichCurveConst()->Eval(Progress);
		const float Pitch = MagPitch.GetRichCurveConst()->Eval(Progress);
		const float Yaw = MagYaw.GetRichCurveConst()->Eval(Progress);
		Rot = Rot * FRotator(Pitch, Yaw, Roll).Quaternion();
	}
	outTransform = FTransform(Rot, ClosestPoint);
	return FMath::Min(Progress, 1.f);
}


void UTVRMagazineWell::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    HandleMagDrop(DeltaTime);
	if(IgnoreMagazine)
	{
		const FVector MagLoc = IgnoreMagazine->GetAttachOrigin()->GetComponentLocation();
		const FVector EjectLoc = GetEjectLocation();
		if(FVector::DistSquared(MagLoc, EjectLoc) > 10.f)
		{
			IgnoreMagazine = nullptr;
			GetWorld()->GetTimerManager().ClearTimer(IgnoreMagTimer);
		}
	}
}
