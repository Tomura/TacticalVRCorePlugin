// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRTriggerComponent.h"
#include "Weapon/TVRGunBase.h"
#include "Player/TVRCharacter.h"
#include "Weapon/Attachments/WPNA_UnderbarrelWeapon.h"
#include "GripMotionControllerComponent.h"

// Sets default values for this component's properties
UTVRTriggerComponent::UTVRTriggerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	TriggerAxis = 0.f;
	
	TriggerActivate = 0.4f;
	TriggerReset = 0.2f;
	bTriggerNeedsReset = false;

	bDualStageTrigger = false;
	TriggerActivate2 = 0.8f;
	TriggerReset2 = 0.65f;
	bTriggerNeedsReset2 = false;
}

ATVRGunBase* UTVRTriggerComponent::GetGunOwner() const
{
	if(GetOwner())
	{
		if(auto GunOwner = Cast<ATVRGunBase>(GetOwner()))
		{
			return GunOwner;
		}
		if(auto AttachmentOwner = Cast<ATVRWeaponAttachment>(GetOwner()))
		{
			return AttachmentOwner->GetGunOwner();
		}
	}
	return nullptr;
}

ATVRCharacter* UTVRTriggerComponent::GetCharacterOwner() const
{
	return GetGunOwner() ? GetGunOwner()->GetVRCharacterOwner() : nullptr;
}

APlayerController* UTVRTriggerComponent::GetOwnerPlayerController() const
{
	if(AController* MyController = GetCharacterOwner()->Controller)
	{
		return Cast<APlayerController>(MyController);
	}
	return nullptr;
}

bool UTVRTriggerComponent::IsOwnerLocallyControlled() const
{
	return GetGunOwner()->IsOwnerLocalPlayerController();
}

void UTVRTriggerComponent::ActivateTrigger(UGripMotionControllerComponent* ActivatingController)
{
	if(ActivatingController)
	{
		UsingController = ActivatingController;
		SetComponentTickEnabled(true);
	}
}

void UTVRTriggerComponent::DeactivateTrigger()
{
	if(UsingController)
	{
		EControllerHand HandType;
		UsingController->GetHandType(HandType);
		APawn* PawnOwner = Cast<APawn>(UsingController->GetOwner());
		if(PawnOwner && PawnOwner->IsLocallyControlled())
		{
			if(APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController()))
			{
				PC->SetHapticsByValue(0.f, 0.f, HandType);
			}
		}
	}	
	SetComponentTickEnabled(false);
	UsingController = nullptr;
}


// Called when the game starts
void UTVRTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UTVRTriggerComponent::GetParentTriggerAxis() const
{
	if(UsingController)
	{
		ACharacter* CharacterOwner = Cast<ACharacter>(UsingController->GetOwner());
		EControllerHand HandType;
		UsingController->GetHandType(HandType);
		const FName AxisName = HandType == EControllerHand::Left ? FName("TriggerAxis_L") : FName("TriggerAxis_R");
		return CharacterOwner->GetInputAxisValue(AxisName);
	}
	return 0.f;
}

// Called every frame
void UTVRTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!IsOwnerLocallyControlled())
	{
		return;
	}
	
	const float PrevTriggerAxis = TriggerAxis;
	if(UsingController)
	{
		TriggerAxis = GetParentTriggerAxis();
		
		EControllerHand HandType;
		UsingController->GetHandType(HandType);
		APawn* PawnOwner = Cast<APawn>(UsingController->GetOwner());
		if(PawnOwner && PawnOwner->IsLocallyControlled())
		{
			if(APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController()))
			{
				if(bTriggerNeedsReset)
				{
					PC->SetHapticsByValue(0.f, 0.f, HandType);
				}
				else
				{
					const float ClosenessToWall = 1 - FMath::Max(TriggerActivate - TriggerAxis, 0.f)/TriggerActivate;
					const float DeltaTrigger = (TriggerAxis - PrevTriggerAxis)/DeltaTime;
					const float DeltaTriggerDeadZone = 0.01f*ClosenessToWall;
					PC->SetHapticsByValue(0.2f, FMath::Clamp((DeltaTrigger-DeltaTriggerDeadZone)*0.5f*ClosenessToWall, 0.f, 0.1f), HandType);
				}
			}
		}

		if(PrevTriggerAxis != TriggerAxis)
		{
			if(TriggerAxis - PrevTriggerAxis > 0.f)
			{
				// Trigger Increases
				if(!bTriggerNeedsReset && TriggerAxis > TriggerActivate)
				{
					bTriggerNeedsReset = true;
					if(OnTriggerActivate.IsBound())
					{						
						OnTriggerActivate.Broadcast();
					}
				}
				
				if(bDualStageTrigger && !bTriggerNeedsReset2 && TriggerAxis > TriggerActivate2)
				{
					bTriggerNeedsReset2 = true;
					if(OnSecondTriggerStageActivate.IsBound())
					{						
						OnSecondTriggerStageActivate.Broadcast();
					}
				}
			}
			else
			{
				// Trigger Decreases
				if(bTriggerNeedsReset && TriggerAxis < TriggerReset)
				{
					bTriggerNeedsReset = false;
					if(OnTriggerReset.IsBound())
					{						
						OnTriggerReset.Broadcast();
					}
				}
				if(bDualStageTrigger && bTriggerNeedsReset2 && TriggerAxis < TriggerReset2)
				{
					bTriggerNeedsReset2 = false;
					if(OnSecondTriggerStageReset.IsBound())
					{						
						OnSecondTriggerStageReset.Broadcast();
					}
				}
			}
		}
	}
	else if(TriggerAxis != 0.f)
	{
		TriggerAxis = 0.f;
	}
}

