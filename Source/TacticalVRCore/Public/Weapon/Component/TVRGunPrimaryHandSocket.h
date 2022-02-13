// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Grippables/HandSocketComponent.h"
#include "TVRGunPrimaryHandSocket.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TACTICALVRCORE_API UTVRGunPrimaryHandSocket : public UHandSocketComponent
{
	GENERATED_BODY()
public:
	UTVRGunPrimaryHandSocket(const FObjectInitializer& OI);
protected:
	UPROPERTY(Category="Hand Animation", EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* HandAnimTriggerSafe;
	UPROPERTY(Category="Hand Animation", EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* HandAnimTriggerTouch;
	UPROPERTY(Category="Hand Animation", EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* HandAnimTriggerPressed;
};
