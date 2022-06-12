// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/TVRPlayerController.h"

#include "Settings/TVRCoreGameplaySettings.h"
#include "Components/TVRGunHapticsComponent.h"


void ATVRPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Apply Settings to make sure
	UTVRCoreGameplaySettings::Get()->ApplySettings();
}

UTVRGunHapticsComponent* ATVRPlayerController::GetGunHapticsComponent_Implementation() const
{
	return FindComponentByClass<UTVRGunHapticsComponent>();
}
