// This file is covered by the LICENSE file in the root of this plugin.

#include "Libraries/TVRFunctionLibrary.h"
#include "VRGripInterface.h"
#include "VRExpansionFunctionLibrary.h"

bool UTVRFunctionLibrary::ValidateGameplayTag(UObject* ObjectToCheck, const FGameplayTag& BaseTag, const FGameplayTag& GameplayTag, const FGameplayTag& DefaultTag)
{
    if(GameplayTag.IsValid())
    {
        IGameplayTagAssetInterface* InterfaceToCheck = Cast<IGameplayTagAssetInterface>(ObjectToCheck);
        if(InterfaceToCheck != nullptr)
        {
            if(InterfaceToCheck->HasMatchingGameplayTag(BaseTag))
            {
                return InterfaceToCheck->HasMatchingGameplayTag(GameplayTag);
            }
        }
    }
    return (GameplayTag == DefaultTag);
}

bool UTVRFunctionLibrary::ValidateGameplayTagContainer(UObject* ObjectToCheck, const FGameplayTag& BaseTag,
    const FGameplayTagContainer& Tags, const FGameplayTag& DefaultTag)
{
    IGameplayTagAssetInterface* TagAsset = Cast<IGameplayTagAssetInterface>(ObjectToCheck);
    if(TagAsset)
    {
        if(TagAsset->HasMatchingGameplayTag(BaseTag))
        {
            FGameplayTagContainer ObjTags;
            TagAsset->GetOwnedGameplayTags(ObjTags);
            return UVRExpansionFunctionLibrary::MatchesAnyTagsWithDirectParentTag(BaseTag, Tags,ObjTags);
        }
    }
    return Tags.HasTag(DefaultTag);
}

bool UTVRFunctionLibrary::IsObjectGripType(UObject* Object, bool bIsLargeGrip)
{
    if(Object->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
    {
        IGameplayTagAssetInterface* TagAsset = Cast<IGameplayTagAssetInterface>(Object);
        if(TagAsset)
        {
	        const FName GripTagSmall = FName("GripType.Small");
	        const bool bObjIsSmall = TagAsset->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(GripTagSmall));
	        if(!bIsLargeGrip && bObjIsSmall)
	        {
	        	return true;
	        }
        	if(bIsLargeGrip && bObjIsSmall)
	        {
		        const FName GripTagLarge = FName("GripType.Large");
            	const bool bObjIsAlsoLarge = TagAsset->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(GripTagLarge));
            	return bObjIsAlsoLarge;
	        }
        }
    }
    return bIsLargeGrip;
}

float UTVRFunctionLibrary::GetDistanceAlongSplineClosestToWorldLocation(
    const USplineComponent* Spline, const FVector& Location)
{
    const float Key = Spline->FindInputKeyClosestToWorldLocation(Location);
    const float Res = Key / (Spline->GetNumberOfSplinePoints()-1) * Spline->GetSplineLength();    
    return Res;
}

UObject* UTVRFunctionLibrary::GetClassDefaultObject(UClass* Class)
{
	return Class->GetDefaultObject();
}
