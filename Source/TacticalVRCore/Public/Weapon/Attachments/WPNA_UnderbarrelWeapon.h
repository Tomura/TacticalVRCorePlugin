// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "Weapon/Attachments/WPNA_Foregrip.h"
#include "WPNA_UnderbarrelWeapon.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnderbarrelWeaponActionEvent);
/**
* 
*/
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_UnderbarrelWeapon : public AWPNA_ForeGrip
{
	GENERATED_BODY()

	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Movables;

	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class UTVRTriggerComponent* TriggerComponent;

	UPROPERTY(Category = "Weapon Attachment", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class UTVRGunFireComponent* FiringComponent;
	
public:
	
	AWPNA_UnderbarrelWeapon(const FObjectInitializer& OI);
	virtual void BeginPlay() override;

	virtual bool GetGripSlot(
		const FVector& WorldLocation, 
		class UGripMotionControllerComponent* CallingController,
		FTransform& OutTransform,
		FName& OutSlotName
	) const override;

	virtual UHandSocketComponent* GetHandSocket_Implementation(FName SlotName) const override;

	virtual void OnGripped(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, bool bIsSecondaryGrip) override;
	virtual void OnReleased(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bIsSecondaryGrip) override;
	
	bool OnMagReleasePressed();	
    bool OnMagReleaseReleased(); 
	bool OnBoltReleasePressed();	
    bool OnBoltReleaseReleased();
	
	/** Returns true if the mag release is pressed */ 
	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
	bool IsMagReleasePressed() const {return bMagReleasePressed;}

	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
	virtual bool IsGripped() const;
	
	UFUNCTION(Category = "Weapon Attachment", BlueprintCallable)
	UTVRTriggerComponent* GetTriggerComponent() const {return TriggerComponent;}

	
	UFUNCTION(Category = "ForeGrip", BlueprintNativeEvent, BlueprintCallable)
	class UHandSocketComponent* GetSecondaryHandSocket() const;
	virtual class UHandSocketComponent* GetSecondaryHandSocket_Implementation() const {return nullptr;}
	
protected:
	UFUNCTION(Category="Weapon Attachment", BlueprintNativeEvent)
	void OnStartFire();
	virtual void OnStartFire_Implementation();
	
	UFUNCTION(Category="Weapon Attachment", BlueprintNativeEvent)
	void OnStopFire();
	virtual void OnStopFire_Implementation();

	UFUNCTION(Category="Weapon Attachment", BlueprintNativeEvent)
	void OnFire();
	virtual void OnFire_Implementation();
	
protected:
	bool bMagReleasePressed;

	/** Event that is fired when the mag release is pressed. The action will count as unhandled when this event is not bound to anything */
	UPROPERTY(Category = "Weapon Attachment", BlueprintAssignable)
	FUnderbarrelWeaponActionEvent OnMagReleasePressedEvent;
	
	/** Event that is fired when the mag release is released. The action will count as unhandled when this event is not bound to anything */
	UPROPERTY(Category = "Weapon Attachment", BlueprintAssignable)
	FUnderbarrelWeaponActionEvent OnMagReleaseReleasedEvent;
	
	/** Event that is fired when the bolt release is pressed. The action will count as unhandled when this event is not bound to anything */
	UPROPERTY(Category = "Weapon Attachment", BlueprintAssignable)
	FUnderbarrelWeaponActionEvent OnBoltReleasePressedEvent;
	
	/** Event that is fired when the bolt release is released. The action will count as unhandled when this event is not bound to anything */
	UPROPERTY(Category = "Weapon Attachment", BlueprintAssignable)
	FUnderbarrelWeaponActionEvent OnBoltReleaseReleasedEvent;
	
	UPROPERTY(Category = "Weapon Attachment|Recoil", EditDefaultsOnly)
	FVector RecoilImpulse;
	UPROPERTY(Category = "Weapon Attachment|Recoil", EditDefaultsOnly)
	FVector AngularRecoilImpulse;

};
