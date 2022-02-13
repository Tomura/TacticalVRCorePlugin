// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "TVRGunBase.h"
#include "TVRGunWithChild.generated.h"

UCLASS()
class TACTICALVRCORE_API ATVRGunWithChild : public ATVRGunBase
{
	GENERATED_BODY()

public:
	/**
	 * Constructs an instance of this class and sets the default values.
	 * Spawn the actor instead of constructing it.
	 */
	ATVRGunWithChild(const FObjectInitializer& OI);

	/**
	 * Called when the actor is spawned or when the level begins
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the actor is destroyed
	 */
	virtual void Destroyed() override;

	virtual void SetOwner(AActor* NewOwner) override;


	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) override;
	
	/**
	 * override of on start fire to prevent any firing logic from this gun. The child gun will do everything
	 */
	virtual void OnStartFire() override {}
	virtual void OnStopFire() override {}
	virtual bool CanStartFire() const override;

	UFUNCTION(BlueprintNativeEvent, Category = "GunWithChild")
	bool CanPressMagRelease(const FVector& PressLocation) const;
	virtual bool CanPressMagRelease_Implementation(const FVector& PressLocation) const { return false; }

	UFUNCTION()
	virtual void OnMagReleasePressedHovered(class UGripMotionControllerComponent* Controller);
	virtual void OnMagReleaseReleasedHovered();
	
	
	UFUNCTION(Category= "GunWithChild", BlueprintCallable)
	virtual void SetChildGun(ATVRGunBase* NewChild);

	virtual void AddRecoil() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "GunWithChild")
	class UTVRHoverInputVolume* GetChildHoverInputComp() const;

protected:
	UPROPERTY(Category = "GunWithChild", BlueprintReadOnly);
	ATVRGunBase* ChildGun;

	UFUNCTION()
	virtual void OnChildGunDestroyed(AActor* DestroyedActor);

	FTimerHandle MagReleaseTimer;
};
