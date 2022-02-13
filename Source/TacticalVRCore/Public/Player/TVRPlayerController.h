// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "VRPlayerController.h"
#include "TVRPlayerController.generated.h"

enum class ForceTubeVRChannel : uint8; // forward declaration of ForceTubeVRChannel

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API ATVRPlayerController : public AVRPlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	
	/**
	 * Perfroms a Kick with ForceTubeVR.
	 * Prefferably call it only when you are sure, that you are executing code on the onwer,
	 * but for additional safety, so that it accidentally doesn't fire on any other simulating client
	 * it is marked unreliable, client
	 * @param KickStregth Strength of the Force Tube Kick
	 */
	UFUNCTION(Unreliable, Client, BlueprintCallable, Category = "Force Tube VR")
	void ClientForceTubeKick(uint8 KickStregth, ForceTubeVRChannel Channel);
};
