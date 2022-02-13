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

	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* GripSlot;

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

	virtual USceneComponent* GetGripSlotComponent() const { return GripSlot; }
	UFUNCTION(Category = "ForeGrip", BlueprintNativeEvent, BlueprintCallable)
	class UHandSocketComponent* GetPrimaryHandSocket() const;
	virtual class UHandSocketComponent* GetPrimaryHandSocket_Implementation() const {return nullptr;}

	virtual class UHandSocketComponent* GetHandSocket_Implementation(FName SlotName) const override;
};
