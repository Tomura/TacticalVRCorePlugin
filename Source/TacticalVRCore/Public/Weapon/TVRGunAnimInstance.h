// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TVRGunAnimInstance.generated.h"

enum class ETVRFireMode : uint8;
enum class ETVRLeftRight : uint8;

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API UTVRGunAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend class ATVRGunBse;
public:
    UTVRGunAnimInstance(const FObjectInitializer& OI);
    
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual float GetSelectorTargetValue() const;
	
    class ATVRGunBase* GetGunOwner() const;

	UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float Hammer;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float Bolt;
	UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float BoltDistance;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float ChargingHandle;
	UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float ChargingHandleDistance;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
    ETVRFireMode FiringMode;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
    float Trigger;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
    bool bMagazineReleasePressed;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
    bool bBoltReleasePressed;
    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
    bool bIsBoltLocked;

    UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
    bool bIsChargingHandleGrabbed;

	UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float SelectorValue;
	
	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	float SelectorValueSingle;
	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	float SelectorValueBurst;
	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	float SelectorValueAuto;

	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	float SelectorLerpSpeed;
	
	bool bSelectorInitialized;

	UPROPERTY(Category = "Gun", BlueprintReadWrite, EditAnywhere)
	float ChargingHandleStroke;

	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	float MagInsertionProgress;

	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	bool bFirstRoundEjected;
	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	bool bOpenDustCover;
	
	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	ETVRLeftRight ChargingHandleGrabType;
};
