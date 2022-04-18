// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRChargingHandle.h"

#include "TacticalCollisionProfiles.h"
#include "Components/AudioComponent.h"

UTVRChargingHandle::UTVRChargingHandle(const FObjectInitializer& OI) : Super(OI)
{
	SetGenerateOverlapEvents(true);
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;

	SetHiddenInGame(true);
	SetCollisionProfileName(COLLISION_CHARGING_HANDLE);
	
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
	bIsReciprocating = false;
	
	bShouldPlayBackSound = false;
	bShouldPlayCloseSound = false;
	GrabLocation = ETVRLeftRight::None;
}

void UTVRChargingHandle::BeginPlay()
{
	Super::BeginPlay();
	if(ChargingHandleSoundCue && !AudioComponent)
	{
		AudioComponent = NewObject<UAudioComponent>(GetOwner());
		AudioComponent->bAutoActivate = false;
		AudioComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		AudioComponent->SetSound(ChargingHandleSoundCue);
	}
	
	InitialRelativeTransform = GetRelativeTransform();}

void UTVRChargingHandle::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

	if(!VRGripInterfaceSettings.bIsHeld)
	{
		ChargingHandleSpeed -= CurrentProgress * ChargingHandleStiffness * DeltaTime;
		const float NewProgress = CurrentProgress + ChargingHandleSpeed * DeltaTime;
		if(NewProgress > KINDA_SMALL_NUMBER)
		{			
			Execute_SetProgress(this, NewProgress);
			if(CurrentProgress >= 0.15f)
			{
				bShouldPlayCloseSound = true;
			}
		}
		else
		{
			Execute_SetProgress(this, 0.f);
			PlayCloseSound();
			ChargingHandleSpeed = 0.f;
			SetComponentTickEnabled(false);
		}
		if(EventTick.IsBound())
		{
			EventTick.Broadcast(DeltaTime);
		}
	}
	else
	{
		// we do not need this tick if the component is held
		ChargingHandleSpeed = 0.f;
		SetComponentTickEnabled(false);
	}
}

void UTVRChargingHandle::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController,
                                               const FBPActorGripInformation& GripInformation)
{
	Super::OnGrip_Implementation(GrippingController, GripInformation);	
	SetComponentTickEnabled(false);

	ChargingHandleSpeed = 0.f;
	InitialProgress = CurrentProgress;
	InitialGripTransform = GetRelativeGripTransform(GrippingController);

	GrabLocation = InitialGripTransform.GetLocation().Y > 0 ?  ETVRLeftRight::Left : ETVRLeftRight::Right;
}

void UTVRChargingHandle::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController,
	const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	Super::OnGripRelease_Implementation(ReleasingController, GripInformation, bWasSocketed);
	SetComponentTickEnabled(true);
	GrabLocation = ETVRLeftRight::None;
}

void UTVRChargingHandle::TickGrip_Implementation(UGripMotionControllerComponent* GrippingController,
	const FBPActorGripInformation& GripInformation, float DeltaTime)
{
	const float PreviousProgress = CurrentProgress;
	Super::TickGrip_Implementation(GrippingController, GripInformation, DeltaTime);
	const FTransform CurrentGripTransform = GetRelativeGripTransform(GrippingController);
	const float DeltaX = CurrentGripTransform.GetLocation().X - InitialGripTransform.GetLocation().X;
	SetProgress_Implementation(InitialProgress + DeltaX/MaxDeflection);

	ChargingHandleSpeed = (PreviousProgress - CurrentProgress) / DeltaTime / MaxDeflection;

	
	if(EventTickGrip.IsBound())
	{
		EventTickGrip.Broadcast(GrippingController, GripInformation, DeltaTime);
	}
		
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
		PlayRackBackSound();
	}
	if(CurrentProgress <= 0.f)
	{
		PlayCloseSound();
	}

}

FTransform UTVRChargingHandle::GetRelativeGripTransform(UGripMotionControllerComponent* GrippingController) const
{
	return GrippingController->GetPivotTransform() * GetOwner()->GetActorTransform().Inverse() * InitialRelativeTransform.Inverse();
}

void UTVRChargingHandle::LockChargingHandle_Implementation(float LockProgress)
{
	if(bIsReciprocating && !bIsLocked)
	{
		LockedProgress = LockProgress;
		bIsLocked = true;
		Execute_SetProgress(this, LockProgress);
		SetComponentTickEnabled(false);
	}
}

void UTVRChargingHandle::UnlockChargingHandle_Implementation()
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

void UTVRChargingHandle::SetProgress_Implementation(float Value)
{
	CurrentProgress = FMath::Clamp(
		Value,
		bIsLocked ? LockedProgress : 0.f,
		1.f);
	SetRelativeLocation(InitialRelativeTransform.TransformPositionNoScale(FVector(CurrentProgress*MaxDeflection, 0.f, 0.f)));
}

void UTVRChargingHandle::OnBoltClosed_Implementation() const
{
	if(!Execute_IsReciprocating(this) && AudioComponent)
	{
		AudioComponent->Stop();
		// const float VolumeBase = FMath::Abs(ChargingHandleSpeed * MaxDeflection) * 0.5f;
		AudioComponent->SetVolumeMultiplier(1.f);
		AudioComponent->SetBoolParameter(FName("Back"), false);
		// AudioComponent->SetBoolParameter(FName("Pump"), true);
		AudioComponent->Play();
	}
}

void UTVRChargingHandle::PlayRackBackSound()
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

void UTVRChargingHandle::PlayCloseSound()
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
