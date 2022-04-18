// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/TVRHandSocketInterface.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_Barrel.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API AWPNA_Barrel : public ATVRWeaponAttachment, public ITVRHandSocketInterface
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* FiringLocOverride;

public:
	AWPNA_Barrel(const FObjectInitializer& OI);
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void AttachToWeapon(UTVRAttachmentPoint* AttachPoint) override;
	void ModifyMuzzleLocation();

	UFUNCTION(Category="Barrel", BlueprintNativeEvent, BlueprintCallable)
	USceneComponent* GetSecondarySlotComponent() const;	
	virtual USceneComponent* GetSecondarySlotComponent_Implementation() const { return nullptr; }
	
	virtual bool GetGripSlot(const FVector& WorldLocation, class UGripMotionControllerComponent* CallingController, FTransform& OutTransform, FName& OutSlotName) const override;
	virtual class UHandSocketComponent* GetHandSocket_Implementation(FName SlotName) const override;

	/** Function called when sights need to be folded */
	UFUNCTION(Category = "Barrel", BlueprintImplementableEvent)
	void OnFoldSights(bool bShouldFold);
	
protected:
	UPROPERTY(Category="Barrel", EditDefaultsOnly)
	bool bIsSuppressor;
};
