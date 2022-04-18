// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRMagWellComponent.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/TVRMagazine.h"
#include "VRBPDatatypes.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Libraries/TVRFunctionLibrary.h"
#include "Player/TVRCharacter.h"
#include "Sound/SoundCue.h"

#define MAG_AUDIO_StartInsert 0
#define MAG_AUDIO_FullyInserted 1
#define MAG_AUDIO_StartDrop 2
#define MAG_AUDIO_FullyDropped 3

UTVRMagWellComponent::UTVRMagWellComponent(const FObjectInitializer& OI) : Super(OI)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	MagAudioComp = nullptr;
	MagazineSound = nullptr;
	bIsMagFree = false;
    CurrentMagazine = nullptr;
    CachedMagSpline = nullptr;
    MagVelocity = FVector::ZeroVector;
    
    bMagReleasePressed = false;
	bUseCurve = false;
	bWasReleasedByHand = true;
	bNeedsToBeReleasedByHand = false;
	bHasMagRelease = false;
	DefaultMagazineClass = nullptr;
}

void UTVRMagWellComponent::BeginPlay()
{
    Super::BeginPlay();

	if(!CachedMagSpline)
	{
		CachedMagSpline = FindMagSpline();
	}

	if(MagazineSound && !MagAudioComp)
	{
		MagAudioComp = NewObject<UAudioComponent>(GetOwner());
		MagAudioComp->bAutoActivate = false;
		MagAudioComp->SetSound(MagazineSound);
		MagAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	SpawnMagazineAttached();
}

void UTVRMagWellComponent::BeginDestroy()
{
	if(MagAudioComp && !MagAudioComp->IsPendingKill())
	{
		MagAudioComp->DestroyComponent();
	}
	Super::BeginDestroy();
}

void UTVRMagWellComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
    HandleMagDrop(DeltaTime);
}

void UTVRMagWellComponent::SetMagazineCollisionProfile(FName NewProfile)
{
	Super::SetMagazineCollisionProfile(NewProfile);
	if(ATVRMagazine* Mag = GetCurrentMagazine())
	{
		Mag->SetCollisionProfile(NewProfile);
	}
}

bool UTVRMagWellComponent::CanBoltLock() const
{
	return HasFullyInsertedMagazine() && GetCurrentMagazine()->IsEmpty();
}

bool UTVRMagWellComponent::IsEmpty() const
{
	if(HasFullyInsertedMagazine())
	{
		return GetCurrentMagazine()->IsEmpty();
	}
	return true; // really? todo: check behavior without mag
}

float UTVRMagWellComponent::GetAmmoInsertProgress()
{
	if(const ATVRMagazine* const Mag = GetCurrentMagazine())
	{
		return Mag->MagInsertPercentage;
	}
	return  0.f;
}

void UTVRMagWellComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ATVRGunBase* Gun = GetGunOwner();
    if(Gun == nullptr || GetMagSpline() == nullptr)
    {
        return;
    }

    if(CanInsertMag())
    {
        ATVRMagazine* Mag = Cast<ATVRMagazine>(OtherActor);
        if(Mag != nullptr && IsAllowedMagType(Mag->GetClass()) && Mag->VRGripInterfaceSettings.bIsHeld)
        {
			StartInsertMagazine(Mag);
        }
    }
}

USplineComponent* UTVRMagWellComponent::GetMagSpline() const
{
    if(CachedMagSpline) // Most of the time this should be valid, but in case the spline component is destroyed we need to detect it
    {
        return CachedMagSpline;
    }
	return FindMagSpline();
}

USplineComponent* UTVRMagWellComponent::FindMagSpline() const
{
	TArray<USceneComponent*> Children;
	GetChildrenComponents(true, Children);
	if(Children.Num() > 0)
	{
		for(USceneComponent* TestComp : Children)
		{
			if(USplineComponent* TestSpline = Cast<USplineComponent>(TestComp))
			{
				return TestSpline;
			}
		}
	}
	return nullptr;
}

void UTVRMagWellComponent::RepositionMagazine()
{
    ATVRGunBase* Gun = GetGunOwner();
    if(Gun != nullptr && GetMagSpline() != nullptr && HasMagazine())
    {
        const FTransform SplineTransform = GetMagSpline()->GetTransformAtTime(0.f, ESplineCoordinateSpace::World, false);
        CurrentMagazine->SetMagazineOriginToTransform(TransformSplineToMagazineCoordinates(SplineTransform));
    }
}

void UTVRMagWellComponent::HandleMagDrop(float DeltaSeconds)
{
    ATVRGunBase* Gun = GetGunOwner();
    const USplineComponent* MyMagSpline = GetMagSpline();
    if(Gun == nullptr || MyMagSpline == nullptr)
    {
        return;
    }
    
    if(HasMagazine() && bIsMagFree)
    {
        const bool bEjectMag = ShouldEjectMag();
        const bool bInsertMag = ShouldInsertMag();
        
        if(!bEjectMag && !bInsertMag)
        {
            if(GetCurrentMagazine()->VRGripInterfaceSettings.bIsHeld)
            {
            	bWasReleasedByHand = true;
                MagVelocity = FVector::ZeroVector;
                UGripMotionControllerComponent* GrippingHand = CurrentMagazine->VRGripInterfaceSettings.HoldingControllers[0].HoldingController;               
                const FTransform HandTransform = GrippingHand->GetComponentTransform();
                const FTransform MagSlot = CurrentMagazine->GetGripSlotTransform(GrippingHand);
                const FTransform MagOrigin = CurrentMagazine->GetAttachOrigin()->GetComponentTransform();
                const FTransform QueryTransform = MagOrigin * MagSlot.Inverse() * HandTransform;
                
                FTransform FoundTransform;
            	GetSplineTransform(QueryTransform.GetLocation(), FoundTransform);
                const FTransform NewTransform = TransformSplineToMagazineCoordinates(FoundTransform);

                MagVelocity = (FoundTransform.GetLocation() - MagOrigin.GetLocation())/DeltaSeconds;
                CurrentMagazine->SetMagazineOriginToTransform(NewTransform);
                CurrentMagazine->MagInsertPercentage =
                    UTVRFunctionLibrary::GetDistanceAlongSplineClosestToWorldLocation(MyMagSpline, NewTransform.GetLocation())/MyMagSpline->GetSplineLength();
            }
            else if(!bNeedsToBeReleasedByHand || bWasReleasedByHand)
            {
                const float DropAcceleration = GetWorld()->GetGravityZ();
                MagVelocity = MagVelocity + FVector::UpVector * DropAcceleration * DeltaSeconds;
                
                const FVector MagLoc = GetCurrentMagazine()->GetAttachOrigin()->GetComponentLocation();
                const FVector ExternalVelocity = Gun->GetStaticMeshComponent()->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
                const FVector DesiredMagLocInternal = MagLoc + (MagVelocity) * DeltaSeconds;
                const FVector DesiredMagLoc = DesiredMagLocInternal - ExternalVelocity * DeltaSeconds;
                

                // const FTransform SplineTransform = MyMagSpline->GetTransformAtDistanceAlongSpline(MagDropDistance, ESplineCoordinateSpace::World, false);
                FTransform SplineTransform;
            	GetSplineTransform(DesiredMagLoc, SplineTransform);
                const FVector SplineInternalLoc = MyMagSpline->FindLocationClosestToWorldLocation(DesiredMagLocInternal, ESplineCoordinateSpace::World);
                const FTransform NewTransform = TransformSplineToMagazineCoordinates(SplineTransform);

                MagVelocity = (SplineInternalLoc - MagLoc)/DeltaSeconds;                
                CurrentMagazine->SetMagazineOriginToTransform(NewTransform);
                CurrentMagazine->MagInsertPercentage =
                    UTVRFunctionLibrary::GetDistanceAlongSplineClosestToWorldLocation(MyMagSpline, NewTransform.GetLocation())/MyMagSpline->GetSplineLength();
            }
        }
        else if(bEjectMag)
        {
            OnMagFullyEjected();
        }
        else
        {
            OnMagFullyInserted();
            CurrentMagazine->MagInsertPercentage = 1.f;
        }
    }
}

void UTVRMagWellComponent::OnMagFullyEjected()
{
    const ATVRGunBase* Gun = GetGunOwner();    
    if(HasMagazine() && Gun)
    {
        ATVRMagazine* OldMag = CurrentMagazine;    
        bIsMagFree= false;
        CurrentMagazine = nullptr;
        
        const USplineComponent* MyMagSpline = GetMagSpline();
        const FVector EjectVelocity = MagVelocity;
        const FVector PhysicsVelocity = Gun->GetStaticMeshComponent()->GetPhysicsLinearVelocity();
        const FVector MagFallDir = -MyMagSpline->GetDirectionAtTime(0.f, ESplineCoordinateSpace::World);
        const FVector A = PhysicsVelocity - (PhysicsVelocity | MagFallDir) * MagFallDir;
        const FVector MagLinearVelocity = A + EjectVelocity;
        const FVector MagAngularVelocity = Gun->GetStaticMeshComponent()->GetPhysicsAngularVelocityInDegrees();
    	
    	if(EventOnMagazineFullyDropped.IsBound())
    	{
    		EventOnMagazineFullyDropped.Broadcast();
    	}
        OldMag->OnMagFullyEjected(MagAngularVelocity, MagLinearVelocity);
    }
    else
    {
        // in this case we only modify the variables
        bIsMagFree= false;
        CurrentMagazine = nullptr;
    }
}

void UTVRMagWellComponent::OnMagDestroyed()
{
	bIsMagFree= false;
	CurrentMagazine = nullptr;
}

bool UTVRMagWellComponent::ShouldEjectMag() const
{
    const USplineComponent* MyMagSpline = GetMagSpline();
    if(MyMagSpline != nullptr && HasMagazine())
    {
        const int32 LastIdx = MyMagSpline->GetNumberOfSplinePoints() - 1;
        const float LastKey = LastIdx;
        const float Key = MyMagSpline->FindInputKeyClosestToWorldLocation(GetCurrentMagazine()->GetAttachOrigin()->GetComponentLocation());
        return (LastKey - Key) < 0.01f; 
    }
    return true;
}

void UTVRMagWellComponent::OnMagFullyInserted()
{
    if(HasMagazine())
    {
    	bWasReleasedByHand = false;
        FTransform SplineTransform;
    	GetSplineTransformAtTime(0.f, SplineTransform);
        CurrentMagazine->SetMagazineOriginToTransform(TransformSplineToMagazineCoordinates(SplineTransform));
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

bool UTVRMagWellComponent::ShouldInsertMag() const
{
    if(IsMagReleasePressed())
    {
        return false;
    }
    
    const USplineComponent* MyMagSpline = GetMagSpline();
    if(MyMagSpline != nullptr && HasMagazine())
    {
        const int32 LastIdx = MyMagSpline->GetNumberOfSplinePoints() - 1;
        const float LastKey = LastIdx;
        const float Key = MyMagSpline->FindInputKeyClosestToWorldLocation(GetCurrentMagazine()->GetAttachOrigin()->GetComponentLocation());
        return Key < 0.01f; 
    }
    return true;
}

bool UTVRMagWellComponent::CanInsertMag() const
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

bool UTVRMagWellComponent::HasMagazine() const
{
    if(IsValid(CurrentMagazine))
    {
        return true;
    }
    return false;
}

bool UTVRMagWellComponent::HasFullyInsertedMagazine() const
{
    if(HasMagazine() && !bIsMagFree)
    {
        return true;
    }
    return false;
}

bool UTVRMagWellComponent::TryReleaseMag()
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

bool UTVRMagWellComponent::CanFeedAmmo() const
{
    // As for now two conditions
    // 1) There needs to be a magazine
    // 2) The magazine needs to be fully attached
    
    return HasFullyInsertedMagazine() && !GetCurrentMagazine()->IsEmpty();
}

TSubclassOf<ATVRCartridge> UTVRMagWellComponent::TryFeedAmmo()
{
	if(CanFeedAmmo() && GetCurrentMagazine()->TryConsumeAmmo())
	{
		return GetCurrentMagazine()->GetCartridgeType();
	}
	return nullptr;
}

bool UTVRMagWellComponent::IsAllowedMagType(UClass* TestClass) const
{
    return AllowedMagazines.Find(TestClass) != INDEX_NONE;
}

ATVRMagazine* UTVRMagWellComponent::GetCurrentMagazine() const
{
    return CurrentMagazine;
}

void UTVRMagWellComponent::OnMagReleasePressed(bool bAlternatePress)
{
	const bool bAlreadyPressed = IsMagReleasePressed();
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

void UTVRMagWellComponent::OnMagReleaseReleased(bool bAlternatePress)
{
	if(!bAlternatePress && bMagReleasePressed)
	{
		bMagReleasePressed = false;
	}
	const bool bStillPressed = IsMagReleasePressed();
	if(!bStillPressed)
	{
		if(EventOnMagReleaseReleased.IsBound())
		{
			EventOnMagReleaseReleased.Broadcast();
		}
	}
}

void UTVRMagWellComponent::OnOwnerGripReleased(ATVRCharacter* OwningChar,UGripMotionControllerComponent* ReleasingHand)
{
	Super::OnOwnerGripReleased(OwningChar, ReleasingHand);
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

bool UTVRMagWellComponent::IsMagReleasePressed() const
{
	return bMagReleasePressed || (GetCurrentMagazine() && GetCurrentMagazine()->IsMagReleasePressed());
}

void UTVRMagWellComponent::GetSplineTransform(const FVector& inLoc, FTransform& outTransform) const
{	
    const USplineComponent* MyMagSpline = GetMagSpline();
	outTransform = FTransform(
		UKismetMathLibrary::MakeRotFromXZ(GetForwardVector(), GetUpVector()),
		MyMagSpline->FindLocationClosestToWorldLocation(inLoc, ESplineCoordinateSpace::World));
	if(bUseCurve)
	{
		const float Key = MyMagSpline->FindInputKeyClosestToWorldLocation(inLoc);
		
		const float Roll = MagRoll.GetRichCurveConst()->Eval(Key);
		const float Pitch = MagPitch.GetRichCurveConst()->Eval(Key);
		const float Yaw = MagYaw.GetRichCurveConst()->Eval(Key);
		outTransform = FTransform(FRotator(Pitch, Yaw, Roll)) * outTransform;
	}
}

void UTVRMagWellComponent::GetSplineTransformAtTime(float inProgress, FTransform& outTransform) const
{
	const USplineComponent* MyMagSpline = GetMagSpline();
	outTransform = FTransform(
		UKismetMathLibrary::MakeRotFromXZ(GetForwardVector(), GetUpVector()),
		MyMagSpline->GetLocationAtTime(0.f, ESplineCoordinateSpace::World));	
	if(bUseCurve)
	{	
		const float Roll = MagRoll.GetRichCurveConst()->Eval(inProgress);
		const float Pitch = MagPitch.GetRichCurveConst()->Eval(inProgress);
		const float Yaw = MagYaw.GetRichCurveConst()->Eval(inProgress);
		outTransform = FTransform(FRotator(Pitch, Yaw, Roll)) * outTransform;
	}
}

ATVRMagazine* UTVRMagWellComponent::SpawnMagazineAttached(TSubclassOf<ATVRMagazine> MagazineClass)
{
	if(GetCurrentMagazine() || !GetMagSpline())
	{
		return nullptr;
	}

	ATVRMagazine* MyMag = nullptr;
	const auto& TempChildren = GetAttachChildren();
	for(const auto TestChild : TempChildren)
	{
		if(auto TestMag = Cast<ATVRMagazine>(TestChild->GetOwner()))
		{
			MyMag = TestMag;
		}
	}
	if(MyMag)
	{
		MyMag->Destroy();
	}
	
	if(const auto World = GetWorld())
	{
		if(const auto Gun = GetGunOwner())
		{
			if(MagazineClass == nullptr)
			{
				if(AllowedMagazines.Num() > 0)
				{
					MagazineClass = AllowedMagazines[0];
				}
				else
				{
					return nullptr;
				}
			}
			if(IsAllowedMagType(MagazineClass) && GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.bAllowDuringConstructionScript = false;
				// in theory we could allow spawn during construction script, but it would require addtional code like child actor components
				
				auto NewMag = World->SpawnActor<ATVRMagazine>(MagazineClass, SpawnParams);
				if(NewMag)
				{
					FTransform SplineTransform;
					GetSplineTransformAtTime(0.f, SplineTransform);
					NewMag->TryAttachToWeapon(Gun->GetStaticMeshComponent(), this, TransformSplineToMagazineCoordinates(SplineTransform));
					CurrentMagazine = NewMag;
					OnMagFullyInserted();
					return NewMag;
				}
			}
		}
	}
	return nullptr;
}

void UTVRMagWellComponent::StartInsertMagazine(ATVRMagazine* MagToInsert)
{
	ATVRGunBase* Gun = GetGunOwner();
	if(Gun == nullptr || GetMagSpline() == nullptr)
	{
		return;
	}
	
	// const FTransform SplineTransform = GetMagSpline()->GetTransformAtTime(0.f, ESplineCoordinateSpace::World, false);
	FTransform SplineTransform;
	GetSplineTransform(MagToInsert->GetAttachOrigin()->GetComponentLocation(), SplineTransform);
	if(MagToInsert->TryAttachToWeapon(Gun->GetStaticMeshComponent(), this, TransformSplineToMagazineCoordinates(SplineTransform)))
	{
		bIsMagFree = true;
		//GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UTVRMagWellComponent::RepositionMagazine);
		CurrentMagazine = MagToInsert;
		//Gun->OnMagazineInserted(Mag);

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

FTransform UTVRMagWellComponent::TransformSplineToMagazineCoordinates(const FTransform& InTransform) const
{
    const FVector NewLoc = InTransform.GetLocation();
    // const FRotator NewRot = UKismetMathLibrary::MakeRotFromZX(-1.f * InTransform.GetRotation().GetForwardVector(), InTransform.GetRotation().GetUpVector());
     const FRotator NewRot = UKismetMathLibrary::MakeRotFromXZ(GetForwardVector(), GetUpVector());
    return FTransform(InTransform.GetRotation(), NewLoc, FVector::OneVector);
}
