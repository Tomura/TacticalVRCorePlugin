// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "Grippables/HandSocketComponent.h"
#include "UObject/Interface.h"
#include "TVRHandSocketInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTVRHandSocketInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TACTICALVRCORE_API ITVRHandSocketInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(Category="HandSocket", BlueprintCallable, BlueprintNativeEvent)
	UHandSocketComponent* GetHandSocket(FName SlotName) const;
	virtual UHandSocketComponent* GetHandSocket_Implementation(FName SlotName) const {return nullptr;}
};
