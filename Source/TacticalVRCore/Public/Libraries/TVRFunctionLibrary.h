// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTags.h"
#include "Components/SplineComponent.h"


#include "TVRFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API UTVRFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @param ObjectToCheck Object that is checked
	 * @param BaseTag Base Tag to check for
	 * @param GameplayTag Specific Gameplay Tag to check for
	 * @param DefaultTag Default Tag to check for
	 */
	UFUNCTION(Category = "Settings", BlueprintCallable, BlueprintPure, meta=(DeprecatedFunction))
	static bool ValidateGameplayTag(
	    class UObject* ObjectToCheck,
	    const struct FGameplayTag& BaseTag,
	    const struct FGameplayTag& GameplayTag,
	    const struct FGameplayTag& DefaultTag
	);


	UFUNCTION(Category = "Settings", BlueprintCallable, BlueprintPure, meta=(DeprecatedFunction))
	static bool ValidateGameplayTagContainer(
		class UObject* ObjectToCheck,
		const struct FGameplayTag& BaseTag,
		const struct FGameplayTagContainer& Tags,
		const struct FGameplayTag& DefaultTag
	);

	
	UFUNCTION(Category = "Settings", BlueprintCallable, BlueprintPure)
	static bool IsObjectGripType(UObject* Object, bool bIsLargeGrip);

    static float GetDistanceAlongSplineClosestToWorldLocation(const USplineComponent* Spline, const FVector& Location);

	UFUNCTION(BlueprintPure, Category = "Utilities", meta = (DeterminesOutputType = "Class"))
	static UObject* GetClassDefaultObject(UClass* Class);	
};
