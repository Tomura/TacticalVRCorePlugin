// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/TVRHandSocketInterface.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_ForeGrip.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_ForeGrip : public ATVRWeaponAttachment, public ITVRHandSocketInterface
{
	GENERATED_BODY()

public:
	AWPNA_ForeGrip(const FObjectInitializer& OI);
	virtual bool GetGripSlot(
		const FVector& WorldLocation,
		class UGripMotionControllerComponent* CallingController,
		FTransform& OutTransform,
		FName& OutSlotName
	) const override;

	virtual void OnGripped(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, bool bIsSecondaryGrip) {}
	virtual void OnReleased(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bIsSecondaryGrip) {}
	
	virtual bool IsGripped() const;

	UFUNCTION(Category = "ForeGrip", BlueprintNativeEvent, BlueprintCallable)
	class UHandSocketComponent* GetPrimaryHandSocket() const;
	virtual class UHandSocketComponent* GetPrimaryHandSocket_Implementation() const;

	virtual class UHandSocketComponent* GetHandSocket_Implementation(FName SlotName) const override;

	virtual float GetRecoilModifier_Implementation() const override;
	
protected:
	UPROPERTY(Category="Foregrip", EditDefaultsOnly)
	float PrimarySlotGripDistance;
	
	UPROPERTY(Category="Foregrip", EditDefaultsOnly)
	float RecoilModifier;
};
