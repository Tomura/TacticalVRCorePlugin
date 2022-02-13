// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TVRAmmoType.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TACTICALVRCORE_API UTVRAmmoType : public UObject
{
	GENERATED_BODY()

public:
	UTVRAmmoType();

	bool IsBuckShot() const;
	
protected:
	UPROPERTY(Category = "Ammo", EditDefaultsOnly)
	bool bIsBuckShot;

	UPROPERTY(Category = "BuckshotCount", EditDefaultsOnly)
	uint8 BuckShotCount;
};
