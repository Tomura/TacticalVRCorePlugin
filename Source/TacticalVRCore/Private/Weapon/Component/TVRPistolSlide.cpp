// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRPistolSlide.h"
#include "TacticalCollisionProfiles.h"
#include "Components/AudioComponent.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Component/TVRGunFireComponent.h"

UTVRPistolSlide::UTVRPistolSlide(const FObjectInitializer& OI) : Super(OI)
{
	SetGenerateOverlapEvents(true);
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;

	SetHiddenInGame(false);
	SetCollisionProfileName(COLLISION_WEAPON);
	
	VRGripInterfaceSettings.FreeDefaultGripType = EGripCollisionType::CustomGrip;
	VRGripInterfaceSettings.SlotDefaultGripType = EGripCollisionType::CustomGrip;
	VRGripInterfaceSettings.SecondaryGripType = ESecondaryGripType::SG_None;
	VRGripInterfaceSettings.LateUpdateSetting = EGripLateUpdateSettings::LateUpdatesAlwaysOff;
	VRGripInterfaceSettings.bSimulateOnDrop = false;
	
    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GripType.Small")));
	InitialProgress = 0.f;
	CurrentProgress = 0.f;
	ChargingHandleSpeed = 0.f;
	MaxDeflection = 8.f;
	ChargingHandleStiffness = 100.f;
	LockedProgress = 0.f;
	
	bIsLocked = false;
	bShouldPlayBackSound = false;
	bShouldPlayCloseSound = false;
}

void UTVRPistolSlide::BeginPlay()
{
	Super::BeginPlay();

	if(SlideSoundCue && !AudioComponent)
	{
		AudioComponent = NewObject<UAudioComponent>(GetOwner());
		AudioComponent->bAutoActivate = false;
		AudioComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		AudioComponent->SetSound(SlideSoundCue);
	}
	
	InitialRelativeTransform = GetRelativeTransform();
	const auto Gun = GetOwner() ? Cast<ATVRGunBase>(GetOwner()) : nullptr;
	if(Gun && Gun->GetFiringComponent())
	{			
		Gun->GetFiringComponent()->OnEndCycle.AddDynamic(this, &UTVRPistolSlide::OnEndFiringCycle);
	}
}

void UTVRPistolSlide::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

	if(!VRGripInterfaceSettings.bIsHeld)
	{
		const auto Gun = GetOwner() ? Cast<ATVRGunBase>(GetOwner()) : nullptr;
		if(Gun && Gun->GetFiringComponent() && Gun->GetFiringComponent()->IsInFiringCooldown())
		{			
			Execute_SetProgress(this, Gun->GetBoltProgress());

			if(CurrentProgress >= 0.15f)
			{
				bShouldPlayCloseSound = true;
			}
		}
		else
		{
			ChargingHandleSpeed -= CurrentProgress * ChargingHandleStiffness * DeltaTime;
			const float NewProgress = CurrentProgress + ChargingHandleSpeed * DeltaTime;
			if(NewProgress > KINDA_SMALL_NUMBER)
			{			
				Execute_SetProgress(this, NewProgress);
			}
			else
			{
				Execute_SetProgress(this, 0.f);
				PlaySlideCloseSound();
				ChargingHandleSpeed = 0.f;
			}
		}
	}
	else
	{
		// we do not need this tick if the component is held
		ChargingHandleSpeed = 0.f;
	}
}

void UTVRPistolSlide::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController,
                                               const FBPActorGripInformation& GripInformation)
{
	Super::OnGrip_Implementation(GrippingController, GripInformation);	
	SetComponentTickEnabled(false);

	ChargingHandleSpeed = 0.f;
	InitialProgress = CurrentProgress;
	InitialGripTransform = GetRelativeGripTransform(GrippingController);
}

void UTVRPistolSlide::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController,
	const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	Super::OnGripRelease_Implementation(ReleasingController, GripInformation, bWasSocketed);
	SetComponentTickEnabled(true);
}

void UTVRPistolSlide::TickGrip_Implementation(UGripMotionControllerComponent* GrippingController,
	const FBPActorGripInformation& GripInformation, float DeltaTime)
{
	const float PreviousProgress = CurrentProgress;
	Super::TickGrip_Implementation(GrippingController, GripInformation, DeltaTime);
	const FTransform CurrentGripTransform = GetRelativeGripTransform(GrippingController);
	const float Delta = InitialGripTransform.GetLocation().Y - CurrentGripTransform.GetLocation().Y;
	SetProgress_Implementation(InitialProgress + Delta/MaxDeflection);
	ChargingHandleSpeed = (CurrentProgress - PreviousProgress) / DeltaTime / MaxDeflection;

	if(CurrentProgress <= 0.85f)
	{
		bShouldPlayBackSound = true;
	}
	if(CurrentProgress >= 0.15f)
	{
		bShouldPlayCloseSound = true;
	}

	if(CurrentProgress >= 1.f)
	{
		PlaySlideBackSound();
	}
	if(CurrentProgress <= 0.f)
	{
		PlaySlideCloseSound();	
	}
}

FTransform UTVRPistolSlide::GetRelativeGripTransform(UGripMotionControllerComponent* GrippingController) const
{
	return GrippingController->GetPivotTransform() * GetOwner()->GetActorTransform().Inverse() * InitialRelativeTransform.Inverse();
}

void UTVRPistolSlide::LockChargingHandle_Implementation(float LockProgress)
{
	if(!bIsLocked)
	{
		LockedProgress = LockProgress;
		bIsLocked = true;		
		Execute_SetProgress(this, LockProgress);
		SetComponentTickEnabled(false);
		// bShouldPlayCloseSound = true;
		// bShouldPlayBackSound = true;
		// PlaySlideBackSound();
	}
}

void UTVRPistolSlide::UnlockChargingHandle_Implementation()
{
	if(bIsLocked)
	{
		bIsLocked = false;
		LockedProgress = 0.f;
		if(CurrentProgress > 0.f)
		{
			SetComponentTickEnabled(true);
		}
	}
}

void UTVRPistolSlide::SetProgress_Implementation(float Value)
{
	CurrentProgress = FMath::Clamp(
		Value,
		bIsLocked ? LockedProgress : 0.f,
		1.f);
	SetRelativeLocation(InitialRelativeTransform.TransformPositionNoScale(FVector(0.f, -CurrentProgress*MaxDeflection, 0.f)));
}


void UTVRPistolSlide::PlaySlideBackSound()
{
	if(bShouldPlayBackSound && AudioComponent)
	{
		bShouldPlayBackSound = false;
		AudioComponent->Stop();
		const float VolumeBase = FMath::Abs(ChargingHandleSpeed * MaxDeflection)* 0.5f;
		AudioComponent->SetVolumeMultiplier(FMath::Clamp(VolumeBase, 0.65f, 1.f));
		AudioComponent->SetBoolParameter(FName("Back"), true);
		// AudioComponent->SetBoolParameter(FName("Pump"), true);
		AudioComponent->Play();
	}
}

void UTVRPistolSlide::PlaySlideCloseSound()
{
	if(bShouldPlayCloseSound && AudioComponent)
	{
		bShouldPlayCloseSound = false;
		AudioComponent->Stop();
		const float VolumeBase = FMath::Abs(ChargingHandleSpeed * MaxDeflection) * 0.5f;
		AudioComponent->SetVolumeMultiplier(FMath::Clamp(VolumeBase, 0.1f, 1.f));
		AudioComponent->SetBoolParameter(FName("Back"), false);
		// AudioComponent->SetBoolParameter(FName("Pump"), true);
		AudioComponent->Play();
	}
}

void UTVRPistolSlide::OnEndFiringCycle()
{
	Execute_SetProgress(this, 0.f);
	ChargingHandleSpeed = 0.f;
}
