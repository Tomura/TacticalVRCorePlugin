// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRPumpAction.h"
#include "Player/TVRCharacter.h"
#include "Weapon/TVRGunBase.h"
#include "TacticalCollisionProfiles.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UTVRPumpAction::UTVRPumpAction(const FObjectInitializer& OI) : Super(OI)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetCollisionProfileName(COLLISION_WEAPON);

	bIsInUse = false;
	bIsLocked = false;
	LockedProgress = 0.f;
	
	PumpProgress = 0.f;
	MaxPumpTravel = 20.f;
	PumpStiffness = 250.f;
	PumpSpeed = 0.f;

	InitialProgress = 0.f;
	AudioComponent = nullptr;
	PumpActionSoundCue = nullptr;
}

void UTVRPumpAction::BeginPlay()
{
	Super::BeginPlay();

	TArray<USceneComponent*> ChildComps;
	if(PumpActionSoundCue && !AudioComponent)
	{
		AudioComponent = NewObject<UAudioComponent>(GetOwner());
		AudioComponent->bAutoActivate = false;
		AudioComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		AudioComponent->SetSound(PumpActionSoundCue);
	}

	InitialRelativeTransform = GetRelativeTransform();
	
	if(GetOwner())
	{
		const auto GunOwner = Cast<ATVRGunBase>(GetOwner());
		if(GunOwner)
		{
			GunOwner->EventOnSecondaryUsed.AddDynamic(this, &UTVRPumpAction::OnStartUse);
			GunOwner->EventOnSecondaryEndUsed.AddDynamic(this, &UTVRPumpAction::OnStopUse);
			GunOwner->EventOnSecondaryGripped.AddDynamic(this, &UTVRPumpAction::OnGripped);
		}
	}
}

void UTVRPumpAction::BeginDestroy()
{
	if(AudioComponent && !AudioComponent->IsPendingKill())
	{
		AudioComponent->DestroyComponent();
	}
	Super::BeginDestroy();
}

void UTVRPumpAction::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);
	const auto GripController = GetGrippingController(); 
	if(bIsInUse && GripController)
	{
		TickGrip(DeltaTime, GripController);
	}
	else
	{
		TickNormal(DeltaTime);
	}
}

void UTVRPumpAction::TickGrip(float DeltaTime, UGripMotionControllerComponent* GrippingController)
{
	const float PreviousProgress = Execute_GetProgress(this);
	const FTransform CurrentGripTransform = GetRelativeGripTransform(GrippingController);
	const float Delta = InitialGripTransform.GetLocation().Y - CurrentGripTransform.GetLocation().Y;
	Execute_SetProgress(this, InitialProgress + Delta/MaxPumpTravel);
	
	PumpSpeed = FMath::Abs(PreviousProgress - PumpProgress)/DeltaTime;

	if(PumpProgress <= 0.85f)
	{
		bShouldPlayRackBackSound = true;
	}
	if(PumpProgress >= 0.15f)
	{
		bShouldPlayCloseSound = true;
	}

	if(PumpProgress >= 1.f)
	{
		PlayPumpBackSound();
	}
	if(PumpProgress <= 0.f)
	{
		PlayPumpCloseSound();	
	}
}

void UTVRPumpAction::TickNormal(float DeltaTime)
{
	if(bIsLocked)
	{		
		PumpSpeed = 0.f;
		SetComponentTickEnabled(false);
	}
	
	if(!Execute_IsInUse(this) && PumpProgress > 0.f)
	{
		PumpSpeed -= PumpProgress * PumpStiffness * DeltaTime;
		const float NewProgress = PumpProgress + PumpSpeed * DeltaTime;
		if(NewProgress > KINDA_SMALL_NUMBER)
		{
			Execute_SetProgress(this, NewProgress);
			
			if(PumpProgress >= 0.15f)
			{
				bShouldPlayCloseSound = true;
			}
		}
		else
		{
			Execute_SetProgress(this, 0.f);
		}
	}
	else
	{
		// we do not need this tick if the component is held
		PlayPumpCloseSound();
		PumpSpeed = 0.f;
		SetComponentTickEnabled(false);	
	}
}

void UTVRPumpAction::OnGripped(UGripMotionControllerComponent* GrippingController,
	const FBPActorGripInformation& GripInfo)
{
	const FTimerDelegate A = FTimerDelegate::CreateLambda([this, GripInfo]
	{
		const auto SecondaryController = Cast<UGripMotionControllerComponent>(GripInfo.SecondaryGripInfo.SecondaryAttachment);
		const auto GripControllerOwner = SecondaryController->GetOwner();
		if(GripControllerOwner)
		{
			if(const auto CharacterOwner = Cast<ATVRCharacter>(GripControllerOwner))
			{
				auto GraspingHand = CharacterOwner->GetGraspingHand(SecondaryController);
				FBPActorGripInformation ModifiedGripInfo = GripInfo;
				ModifiedGripInfo.GrippedObject = this;
				ModifiedGripInfo.GripTargetType = EGripTargetType::ComponentGrip;
				GraspingHand->InitializeAndAttach(ModifiedGripInfo, true, true);
			}
		}
	});
	GetWorld()->GetTimerManager().SetTimer(GrippedTimer, A, 0.1f, false);

}

void UTVRPumpAction::OnStartUse()
{
	if(!Execute_IsInUse(this))
	{
		const auto GripController = GetGrippingController();
		if(GripController)
		{
			InitialProgress = PumpProgress;
			InitialGripTransform = GetRelativeGripTransform(GripController);
			PumpSpeed = 0.f;
			bIsInUse = true;
			SetComponentTickEnabled(true);
		}
	}
}

void UTVRPumpAction::OnStopUse()
{
	if(bIsInUse)
	{
		bIsInUse = false;
		SetComponentTickEnabled(true);
	}
}

void UTVRPumpAction::SetProgress_Implementation(float NewProgress)
{
	PumpProgress = FMath::Clamp(
		NewProgress,
		bIsLocked ? LockedProgress : 0.f,
		1.f);
	SetRelativeLocation(
		InitialRelativeTransform.GetLocation() - FVector(0.f, PumpProgress*MaxPumpTravel, 0.f)
	);

}

void UTVRPumpAction::LockChargingHandle_Implementation(float LockProgress)
{
	if(!bIsLocked)
	{
		bIsLocked = true;
		LockedProgress = LockProgress;
		Execute_SetProgress(this, LockProgress);
	}
}

void UTVRPumpAction::UnlockChargingHandle_Implementation()
{
	if(bIsLocked)
	{
		LockedProgress = 0.f;
		bIsLocked = false;
		SetComponentTickEnabled(true);
	}
}

FTransform UTVRPumpAction::GetRelativeGripTransform(UGripMotionControllerComponent* GrippingController) const
{
	return GrippingController->GetPivotTransform() * GetOwner()->GetActorTransform().Inverse() * InitialRelativeTransform.Inverse();
}

UGripMotionControllerComponent* UTVRPumpAction::GetGrippingController() const
{
	if(GetOwner())
	{
		const auto GunOwner = Cast<ATVRGunBase>(GetOwner());
		if(GunOwner)
		{
			return GunOwner->GetSecondaryController();
		}
	}
	return nullptr;
}

void UTVRPumpAction::PlayPumpBackSound()
{
	if(bShouldPlayRackBackSound && AudioComponent)
	{
		bShouldPlayRackBackSound = false;
		AudioComponent->Stop();
		AudioComponent->SetVolumeMultiplier(FMath::Clamp(FMath::Abs(PumpSpeed) * 0.5f, 0.7f, 1.f));
		AudioComponent->SetBoolParameter(FName("PumpBack"), true);
		AudioComponent->SetBoolParameter(FName("Pump"), true);
		AudioComponent->Play();
	}
}

void UTVRPumpAction::PlayPumpCloseSound()
{
	if(bShouldPlayCloseSound  && AudioComponent)
	{
		bShouldPlayCloseSound = false;
		AudioComponent->Stop();
		AudioComponent->SetVolumeMultiplier(FMath::Clamp(FMath::Abs(PumpSpeed) * 0.5f, 0.1f, 1.f));
		AudioComponent->SetBoolParameter(FName("PumpBack"), false);
		AudioComponent->SetBoolParameter(FName("Pump"), true);
		AudioComponent->Play();
	}
}
