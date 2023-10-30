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

	/**
	 * 
	 * @param GrippingController 
	 * @param GripInformation 
	 */
	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	/**
	 * 
	 * @param ReleasingController 
	 * @param GripInformation 
	 * @param bWasSocketed 
	 */
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) override;

	/**
	 * Tick function that is only called when the component is gripped
	 * @param GrippingController The grip controlelr that is currently gripping this component
	 * @param GripInformation The grip information for the current grip
	 * @param DeltaTime The time between this and the last tick
	 */
	virtual void TickGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, float DeltaTime) override;

	/**
	 * Returns the relative transform of the Grip originating from the initial grip locations
	 * This function is mainly used to track how far the charging handle is pulled back.
	 * As this function calculates and returns a transform it should be used sparingly.
	 * @returns the relative transform of the Grip as a FTransform. 
	 */
	UFUNCTION(Category="Charging Handle", BlueprintCallable)
	virtual FTransform GetRelativeGripTransform(UGripMotionControllerComponent* GrippingController) const;
	
	/** Called when the charging handle should lock back */
	virtual void LockChargingHandle_Implementation(float LockProgress) override;
	
	/** Called when the charging handle should be unlocked */
	virtual void UnlockChargingHandle_Implementation() override;

	/** Changes the progress of the charging handle (0 = compressed, 1 = extended) */
	virtual void SetProgress_Implementation(float Value) override;

	/** Whether the Component is currently used or not */
	
	virtual bool IsInUse_Implementation() const override { return VRGripInterfaceSettings.bIsHeld; }
	
	/**
	 * Whether the Component is locked. Call ITVRChargingHandleInterface::Execute_IsLocked()
	 * @returns the locked status of the charging handle as a boolean
	 */
	virtual bool IsLocked_Implementation() const override { return bIsLocked; }
	
	/**
	 * Returns the Components progress. Call ITVRChargingHandleInterface::Execute_GetProgress()
	 * @returns the Current Progress as a float between 0 and 1.
	 */
	virtual float GetProgress_Implementation() const override { return CurrentProgress; }
	
	/**
	 * Returns the max travel. Call ITVRChargingHandleInterface::Execute_GetMaxTravel()
	 * @returns the maximum deflection in uu.
	 */
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


