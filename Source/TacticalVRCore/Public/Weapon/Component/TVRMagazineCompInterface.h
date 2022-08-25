// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "TVRMagazineCompInterface.generated.h"

/**
 * 
 */
UCLASS(Abstract,
	hideCategories = (ComponentTick, Navigation, Physics, Collision), 
	ClassGroup = (TacticalVR)
)
class TACTICALVRCORE_API UTVRMagazineCompInterface : public UBoxComponent
{
	GENERATED_BODY()

public:
	UTVRMagazineCompInterface(const FObjectInitializer& OI);
	
	virtual void BeginPlay() override;

	/**
	 * @returns Owning Actor cast to ATVRGunBase
	 */
	virtual class ATVRGunBase* GetGunOwner() const;

	/**
	 * Overlap Event. Bound to the BeginOverlap delegate.
	 * @param OverlappedComponent Component that is overlapping (should be this component)
	 * @param OtherActor The actors that is overlapping with the magwell
	 * @param OtherComp The component of OtherActor that is overlapping with this
	 * @param OtherBodyIndex Body Index of the overlapping body
	 * @param bFromSweep Whether the overlap was triggered by a sweep
	 * @param SweepResult Sweep result
	 */
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	virtual bool CanFeedAmmo() const {return false;}
	virtual TSubclassOf<class ATVRCartridge> TryFeedAmmo() {return nullptr;}

	virtual void SetMagazineCollisionProfile(FName NewProfile) {}

	virtual bool CanBoltLock() const {return false;}

	virtual bool IsEmpty() const {return true;}

	virtual void OnMagReleasePressed(bool bAlternatePress = false) {}
	virtual void OnMagReleaseReleased(bool bAlternatePress = false) {}

	UFUNCTION(Category = "Magazine", BlueprintCallable)
	virtual bool IsMagReleasePressed() const {return false;}

	virtual float GetAmmoInsertProgress() {return 0;}

	virtual void OnOwnerGripReleased(class ATVRCharacter* OwningChar, class UGripMotionControllerComponent*);
};
