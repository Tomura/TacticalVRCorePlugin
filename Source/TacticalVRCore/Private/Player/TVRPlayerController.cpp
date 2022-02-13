// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/TVRPlayerController.h"

#include "ForceTubeVRFunctions.h"
#include "Settings/TVRCoreGameplaySettings.h"

void ATVRPlayerController::ClientForceTubeKick_Implementation(uint8 KickStregth, ForceTubeVRChannel Channel)
{ 
	UForceTubeVRFunctions::Kick(KickStregth, Channel);
}

void ATVRPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Apply Settings to make sure
	UTVRCoreGameplaySettings::Get()->ApplySettings();
}
