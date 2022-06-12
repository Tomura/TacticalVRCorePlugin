// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "VRPlayerController.h"
#include "TVRPlayerController.generated.h"

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
	 * This function will try to look for a gun haptics component within the player controller
	 * make sure that a player controller only has one.
	 * @returns the first gun haptics component found
	 */
	UFUNCTION(BlueprintCallable, Category = "Hapctics", BlueprintNativeEvent)
	class UTVRGunHapticsComponent* GetGunHapticsComponent() const;
	virtual class UTVRGunHapticsComponent* GetGunHapticsComponent_Implementation() const;
};
