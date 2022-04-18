// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "TVRChargingHandleInterface.h"
#include "VRBPDatatypes.h"
#include "Components/StaticMeshComponent.h"
#include "TVRPumpAction.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent),  ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVRPumpAction : public UStaticMeshComponent, public ITVRChargingHandleInterface
{
	GENERATED_BODY()

public:
	UTVRPumpAction(const FObjectInitializer& OI);
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void TickGrip(float DeltaTime, class UGripMotionControllerComponent* GrippingController);
	virtual void TickNormal(float DeltaTime);

	UFUNCTION()
	virtual void OnGripped(class UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInfo);
	UFUNCTION()
	virtual void OnStartUse();
	UFUNCTION()
	virtual void OnStopUse();

	virtual float GetProgress_Implementation() const { return PumpProgress; }
	virtual float GetMaxTavel_Implementation() const { return MaxPumpTravel; }
	virtual float GetStiffness_Implementation() const { return PumpStiffness; }
	
	virtual bool IsLocked_Implementation() const { return bIsLocked; }
	virtual bool IsInUse_Implementation() const override { return bIsInUse; }
	
	virtual ETVRLeftRight GetGrabLocation_Implementation() const {return ETVRLeftRight::None;}
	virtual void SetAudioComponent_Implementation(UAudioComponent* NewComp) {AudioComponent = NewComp;}
	
	virtual void SetStiffness_Implementation(float NewStiffness) { PumpStiffness = NewStiffness; }
	virtual void SetMaxTravel_Implementation(float NewTravel) { MaxPumpTravel = NewTravel; }
	virtual void SetProgress_Implementation(float NewProgress);
	
	virtual void LockChargingHandle_Implementation(float LockProgress = -1.f);
	virtual void UnlockChargingHandle_Implementation();
	virtual bool IsReciprocating_Implementation() const {return true;}

	FTransform GetRelativeGripTransform(class UGripMotionControllerComponent* GrippingController) const;
	
	class UGripMotionControllerComponent* GetGrippingController() const;
	
	
protected:
	virtual void PlayPumpBackSound();
	virtual void PlayPumpCloseSound();

	UPROPERTY()
	class UAudioComponent* AudioComponent;
	
	UPROPERTY(Category="PumpAction", EditDefaultsOnly)
	class USoundBase* PumpActionSoundCue;
	
	bool bIsInUse;
	bool bIsLocked;
	float LockedProgress;

	float PumpProgress;
	float MaxPumpTravel;
	float PumpStiffness;
	float PumpSpeed;

	float InitialProgress;
	FTransform InitialGripTransform;
	FTransform InitialRelativeTransform;
	
	FTimerHandle GrippedTimer;
	
	bool bShouldPlayRackBackSound;
	bool bShouldPlayCloseSound;
	
};