// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "GripMotionControllerComponent.h"
#include "Components/ActorComponent.h"
#include "TVRTriggerComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTriggerEvent);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TACTICALVRCORE_API UTVRTriggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(Category="Events", BlueprintAssignable)
	FTriggerEvent OnTriggerActivate;
	UPROPERTY(Category="Events", BlueprintAssignable)
	FTriggerEvent OnTriggerReset;
	UPROPERTY(Category="Events", BlueprintAssignable)
	FTriggerEvent OnSecondTriggerStageActivate;
	UPROPERTY(Category="Events", BlueprintAssignable)
	FTriggerEvent OnSecondTriggerStageReset;
	
protected:
	/** */
	UPROPERTY(Category= "Trigger", EditDefaultsOnly)
	float TriggerActivate;
	UPROPERTY(Category= "Trigger", EditDefaultsOnly)
	float TriggerReset;
	
	bool bTriggerNeedsReset;


	UPROPERTY(Category= "Trigger", EditDefaultsOnly)
	bool bDualStageTrigger;
	
	UPROPERTY(Category= "Trigger|Dual Stage", EditDefaultsOnly, meta=(EditConditon="bDualStageTrigger"))
	float TriggerActivate2;
	
	UPROPERTY(Category= "Trigger|Dual Stage", EditDefaultsOnly, meta=(EditConditon="bDualStageTrigger"))
	float TriggerReset2;
	
	bool bTriggerNeedsReset2;
	
	float TriggerAxis;

	class UGripMotionControllerComponent* UsingController;
	
public:	
	// Sets default values for this component's properties
	UTVRTriggerComponent();

	class ATVRGunBase* GetGunOwner() const;
	class ATVRCharacter* GetCharacterOwner() const;
	class APlayerController* GetOwnerPlayerController() const;
	bool IsOwnerLocallyControlled() const;

	UFUNCTION(Category="Trigger", BlueprintCallable)
	void ActivateTrigger(UGripMotionControllerComponent* ActivatingController);
	
	UFUNCTION(Category="Trigger", BlueprintCallable)
	void DeactivateTrigger();

	UFUNCTION(Category="Trigger", BlueprintCallable)
	float GetTriggerValue() const {return TriggerAxis;}
	UFUNCTION(Category="Trigger", BlueprintCallable)
	float GetTriggerActivateValue() const {return TriggerActivate;}
	UFUNCTION(Category="Trigger", BlueprintCallable)
	float GetTriggerResetValue() const {return TriggerReset;}

	UFUNCTION(Category="Trigger", BlueprintCallable)
	bool DoesTriggerNeedReset() const {return bTriggerNeedsReset;}
	

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	 * Obtains the trigger axis of the controller that is currently using the trigger
	 * @returns the value of the controller's trigger aixs
	 */
	float GetParentTriggerAxis() const;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
