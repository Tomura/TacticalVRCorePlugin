// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "TVRHoverInputVolume.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoveredInputEvent, class UGripMotionControllerComponent*, Controller);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), ShowCategories=(Activation), HideCategories=(Collision, Physics, Rendering, Navigation, ComponentTick) )
class TACTICALVRCORE_API UTVRHoverInputVolume : public USphereComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTVRHoverInputVolume();
	
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnUsed;
	
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnMagReleasePressed;
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnBoltReleaseReleased;
	
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnBoltReleasePressed;
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnMagReleaseReleased;
	
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnLaserPressed;
	UPROPERTY(Category = "Events", BlueprintAssignable)
	FOnHoveredInputEvent EventOnLightPressed;

	UPROPERTY(Category="Input", EditDefaultsOnly)
	float InputBreakOffDistance;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	UGripMotionControllerComponent* MagReleasePressed;
	UPROPERTY()
	UGripMotionControllerComponent* BoltReleasePressed;
	// TMap<UGripMotionControllerComponent*, bool> LaserPressed;
	// TMap<UGripMotionControllerComponent*, bool> LightPressed;

	virtual void CheckEnableTick();
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	virtual void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual bool OnUsed(class UGripMotionControllerComponent* Controller);
	virtual bool OnMagReleasePressed(class UGripMotionControllerComponent* Controller);
	virtual bool OnBoltReleasePressed(class UGripMotionControllerComponent* Controller);
	virtual bool OnMagReleaseReleased(class UGripMotionControllerComponent* Controller);
	virtual bool OnBoltReleaseReleased(class UGripMotionControllerComponent* Controller);
	virtual bool OnLaserPressed(class UGripMotionControllerComponent* Controller);
	virtual bool OnLightPressed(class UGripMotionControllerComponent* Controller);

};
