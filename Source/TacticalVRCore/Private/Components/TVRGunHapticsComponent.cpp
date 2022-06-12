// This file is covered by the LICENSE file in the root of this plugin.


#include "Components/TVRGunHapticsComponent.h"


// Sets default values for this component's properties
UTVRGunHapticsComponent::UTVRGunHapticsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UTVRGunHapticsComponent::BeginPlay()
{
	Super::BeginPlay();
	if(GetOwnerPlayerController()->IsLocalPlayerController())
	{
		LocalBeginPlay();
	}
}

void UTVRGunHapticsComponent::LocalBeginPlay_Implementation()
{
	// do nothing here, but this could for example activate the Haptics Device
}

APlayerController* UTVRGunHapticsComponent::GetOwnerPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

void UTVRGunHapticsComponent::ClientButtstockKick_Implementation(uint8 Strength, float Duration)
{
	ButtstockKick(Strength, Duration);
}

void UTVRGunHapticsComponent::ClientPistolKick_Implementation(uint8 Strength, float Duration, ETVRLeftRight Type)
{
	PistolKick(Strength, Duration, Type);
}

void UTVRGunHapticsComponent::ButtstockKick_Implementation(uint8 Strength, float Duration)
{
	// do nothing, this component is more an interface
}

void UTVRGunHapticsComponent::PistolKick_Implementation(uint8 Strength, float Duration, ETVRLeftRight Type)
{
	// do nothing, this component is more an interface
}

