// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "VRGripInterface.h"
#include "Grippables/GrippableBoxComponent.h"
#include "Weapon/Component/TVRChargingHandleInterface.h"
#include "TVRChargingHandle.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTickChargignHandleGripDelegate, UGripMotionControllerComponent*, GrippingController, const FBPActorGripInformation&, GripInformation, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTickChargingHandleDelegate, float, DeltaTime);

/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent),  ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVRChargingHandle : public UGrippableBoxComponent , public ITVRChargingHandleInterface
{
	GENERATED_BODY()
public:
	UTVRChargingHandle(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) override;

	virtual void TickGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, float DeltaTime) override;

	UFUNCTION(Category="Charging Handle", BlueprintCallable)
	virtual FTransform GetRelativeGripTransform(UGripMotionControllerComponent* GrippingController) const;
	
	virtual void LockChargingHandle_Implementation(float LockProgress) override;
	virtual void UnlockChargingHandle_Implementation() override;
	virtual void SetProgress_Implementation(float Value) override;
	
	virtual bool IsInUse_Implementation() const override { return VRGripInterfaceSettings.bIsHeld; }
	virtual bool IsLocked_Implementation() const override { return bIsLocked; }	
	virtual float GetProgress_Implementation() const override { return CurrentProgress; }
	virtual float GetMaxTavel_Implementation() const override { return MaxDeflection; }
	
	virtual void SetStiffness_Implementation(float NewStiffness) override { ChargingHandleStiffness = NewStiffness; }
	virtual void SetMaxTravel_Implementation(float NewTravel) override { MaxDeflection = NewTravel; }
	
	virtual void SetAudioComponent_Implementation(class UAudioComponent* NewComp) override {}
	
	virtual bool IsReciprocating_Implementation() const {return bIsReciprocating;}

	virtual ETVRLeftRight GetGrabLocation_Implementation() const override { return GrabLocation; }

	virtual void OnBoltClosed_Implementation() const override;
	
	virtual void PlayRackBackSound();
	virtual void PlayCloseSound();

	UPROPERTY(Category="Charging Handle", BlueprintAssignable)
	FTickChargignHandleGripDelegate EventTickGrip;
	UPROPERTY(Category="Charging Handle", BlueprintAssignable)
	FTickChargingHandleDelegate EventTick;
	
protected:
	float InitialProgress;
	FTransform InitialRelativeTransform;
	FTransform InitialGripTransform;
	float ChargingHandleSpeed;
	
	UPROPERTY(Category="Charging Handle", EditDefaultsOnly)
	float ChargingHandleStiffness;

	UPROPERTY(Category="Charging Handle", EditDefaultsOnly)
	float MaxDeflection;
	
	UPROPERTY(Category="Charging Handle", EditDefaultsOnly, BlueprintReadWrite)
	bool bIsReciprocating;

	bool bIsLocked;
	float LockedProgress;

	float CurrentProgress;
	
	bool bShouldPlayBackSound;
	bool bShouldPlayCloseSound;

	ETVRLeftRight GrabLocation;
	
	UPROPERTY()
	class UAudioComponent* AudioComponent;
	
	UPROPERTY(Category="PumpAction", EditDefaultsOnly)
	class USoundBase* ChargingHandleSoundCue;
};


