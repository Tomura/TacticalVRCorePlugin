// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grippables/GrippableStaticMeshComponent.h"
#include "Weapon/Component/TVRChargingHandleInterface.h"
#include "TVRPistolSlide.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent),  ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVRPistolSlide : public UGrippableStaticMeshComponent, public ITVRChargingHandleInterface
{
	GENERATED_BODY()

public:
	UTVRPistolSlide(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) override;

	virtual void TickGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation, float DeltaTime) override;

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
	
	virtual void SetAudioComponent_Implementation(UAudioComponent* NewComp) override {}
	
	virtual bool IsReciprocating_Implementation() const {return true;}

	virtual float GetBoltProgress_Implementation() const override {return CurrentProgress;}

	virtual void PlaySlideBackSound();
	virtual void PlaySlideCloseSound();
protected:

	UFUNCTION()
	virtual void OnEndFiringCycle();
	
	float InitialProgress;
	FTransform InitialRelativeTransform;
	FTransform InitialGripTransform;
	float ChargingHandleSpeed;
	
	UPROPERTY(Category="Charging Handle", EditDefaultsOnly)
	float ChargingHandleStiffness;

	UPROPERTY(Category="Charging Handle", EditDefaultsOnly)
	float MaxDeflection;
	
	bool bIsLocked;
	float LockedProgress;

	float CurrentProgress;

	bool bShouldPlayBackSound;
	bool bShouldPlayCloseSound;
	
	UPROPERTY()
	class UAudioComponent* AudioComponent;
	
	UPROPERTY(Category="PumpAction", EditDefaultsOnly)
	class USoundBase* SlideSoundCue;
};
