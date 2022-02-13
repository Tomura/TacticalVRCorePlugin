// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TVRGripRedirectorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTVRGripRedirectorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TACTICALVRCORE_API ITVRGripRedirectorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(Category="Grip Redirector", BlueprintNativeEvent)
	AActor* GetGripRedirectTarget() const;
	virtual AActor* GetGripRedirectTarget_Implementation() const {return nullptr;}

	UFUNCTION(Category="Grip Redirector", BlueprintNativeEvent)
	bool GetGripSlotOverride(const FVector& GripLocation, bool bIsSecondary, bool& bHadSlot, FTransform& SlotWorldTransform, FName& SlotName, class UGripMotionControllerComponent* Hand) const;
	virtual bool GetGripSlotOverride_Implementation(const FVector& GripLocation, bool bIsSecondary, bool& bHadSlot, FTransform& SlotWorldTransform, FName& SlotName, class UGripMotionControllerComponent* Hand) const { return false; }
};
