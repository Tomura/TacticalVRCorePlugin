// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include <stdbool.h>

#include "CoreMinimal.h"
#include "TVRGunFireComponent.h"
#include "TVRMagazineCompInterface.h"
#include "TVRLoadableBreechComponent.generated.h"


UENUM(BlueprintType)
enum class ETVRLoadableBreechState : uint8
{
	Closed,
	Closing,
	Open,
	Opening
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoadableBreechEvent);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (TacticalVR))
class TACTICALVRCORE_API ULoadableBreechComponent : public UTVRMagazineCompInterface
{
	GENERATED_BODY()
	
	UPROPERTY()
	UAudioComponent* CartridgeInsertAudioComp;
	
	UPROPERTY()
	UAudioComponent* OpenCloseAudioComp;
	
	UPROPERTY()
	class UTVRGunFireComponent* FiringComp;
	
	UPROPERTY()
	class ATVRCartridge* CurrentCartridge;
	
public:
	ULoadableBreechComponent(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ================================================
	// Start MagazineComponentInterface
	// ================================================

	/**
	 * Changes the collision profile of all relevant meshes (anything that isn't a helper but a physical collider)
	 * @param NewProfile Name of the new collision profile
	 */
	virtual void SetMagazineCollisionProfile(FName NewProfile) override;

	/**
	 * @returns True if the conditions are met from the magazine side for the bolt to be locked.
	 */
	virtual bool CanBoltLock() const override;
	
	/**
	 * @returns True if the attached magazine is empty.
	 */
	virtual bool IsEmpty() const override;

	/**
	 * @returns number between 0 and 1, where 1 means magazine is fully inserted
	 */
	virtual float GetAmmoInsertProgress() override;


	virtual void OnOwnerGripReleased(ATVRCharacter* OwningChar, class UGripMotionControllerComponent* ReleasingHand) override;
	/**
	 * @returns True if Ammo can be fed from the magazine to the weapon
	 */
	virtual bool CanFeedAmmo() const override;

	virtual TSubclassOf<class ATVRCartridge> TryFeedAmmo() override;
	

	/**
	 * Overlap Event. Bound to the BeginOverlap delegate.
	 * @param OverlappedComponent Component that is overlapping (should be this component)
	 * @param OtherActor The actors that is overlapping with the magwell
	 * @param OtherComp The component of OtherActor that is overlapping with this
	 * @param OtherBodyIndex Body Index of the overlapping body
	 * @param bFromSweep Whether the overlap was triggered by a sweep
	 * @param SweepResult Sweep result
	 */
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	
	// ================================================
	// End MagazineComponentInterface
	// ================================================
	
	UFUNCTION(Category="Weapon", BlueprintCallable)
	virtual bool OpenBreech();
	
	UFUNCTION(Category="Weapon", BlueprintCallable)
	virtual bool CloseBreech();	
	
	UFUNCTION(Category="Weapon", BlueprintCallable)
	ETVRLoadableBreechState GetBreechOpenState() const {return BreechState;}

	UPROPERTY(Category="Weapon", BlueprintAssignable)
	FLoadableBreechEvent EventOnBeginOpenBreech;
	UPROPERTY(Category="Weapon", BlueprintAssignable)
	FLoadableBreechEvent EventOnOpenedBreech;
	UPROPERTY(Category="Weapon", BlueprintAssignable)
	FLoadableBreechEvent EventOnBeginCloseBreech;
	UPROPERTY(Category="Weapon", BlueprintAssignable)
	FLoadableBreechEvent EventOnClosedBreech;

	float GetOpenDuration() const { return BreechOpenTime > 0.01f ? BreechOpenTime : 0.01f;}

	UFUNCTION(Category="Weapon", BlueprintCallable)
	UAudioComponent* GetCartridgeInsertAudioComp() const {return CartridgeInsertAudioComp;}
	
	UFUNCTION(Category="Weapon", BlueprintCallable)
	UAudioComponent* GetOpenCloseAudioComp() const {return OpenCloseAudioComp; }
	UFUNCTION(Category="Weapon", BlueprintCallable)
	UTVRGunFireComponent* GetFiringComp() const {return FiringComp; }

	ATVRCartridge* GetCurrentCartridge() const {return CurrentCartridge;}
	
	UFUNCTION(Category="Weapon", BlueprintCallable)
	void AssignFiringComp(class UTVRGunFireComponent* NewFiringComp);

	
	UFUNCTION(Category="Weapon", BlueprintCallable)
	bool IsAllowedAmmo(TSubclassOf<ATVRCartridge> CartridgeClass) const;
	
protected:
	virtual void BeginInsertCartridge(class ATVRCartridge* CartridgeToInsert);

	UFUNCTION()
	void OnCartridgeSpent();
	
	UFUNCTION()
	void OnCartridgeGrabbed(class UGripMotionControllerComponent * GrippingController, const struct FBPActorGripInformation& GripInformation);
	
	UFUNCTION()
	void OnCartridgeDropped(class UGripMotionControllerComponent * GrippingController, const struct FBPActorGripInformation& GripInformation, bool bWasSocketed);

	void SetCurrentCartridge(class ATVRCartridge* NewCartridge);

	void AttachCartridge(class ATVRCartridge* Cartridge);
	class ATVRCartridge* DetachCartridge();

	void FullyInsertCartridge();
	void FullyRemoveCartridge();
	
	virtual void OnBreechOpened();
	virtual void OnBreechClosed();
	
protected:
	
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	TArray<TSubclassOf<ATVRCartridge>> AllowedCartridges;
	
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	FTransform TargetTransform;
		
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	float Distance;
	
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	float BreechOpenTime;
	
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	bool bReleaseCartridgeWhenOpened;

	/**
	 * Force (Acceleration in uu/s²) of the inserted Cartridge if there is a ejector.
	 * Make sure that it can overcome gravity (normally 980uu/s²) in case you want reliable ejection.
	 * Needs bReleaseCartridgeWhenOpened to be set to true for the feature to work
	 */
	UPROPERTY(Category="Weapon", EditDefaultsOnly, meta = (EditCondition= "bReleaseCartridgeWhenOpened", ClampMin = "0.0"))
	float EjectorForce;
	
	/**
	 * Sound to play on any events that are related to cartridge insertion.
	 * Use a int parameter (switch) with the name "MagEvent" to make use of automatically switching sounds
	 * use the following convention 0 Start Insertion, 1 Fully Inserted, 2 Start Drop, 3 Fully Dropped
	 */
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	USoundBase* CartridgeInsertSound;
	
	/**
	 * Sound to play when the breech opens or closes.
	 * You can use a bool parameter (branch) with the name "Open" to have different sounds on open and close.
	 */
	UPROPERTY(Category="Weapon", EditDefaultsOnly)
	USoundBase* OpenCloseSound;

	
	float Progress;
	
	ETVRLoadableBreechState BreechState;

	bool bCanRemoveCartridge = false;
	bool bCanFullyInsertCartridge = false;
	bool bCartridgeIsLocked = false;
	
	float CartridgeSpeed;

	FTimerHandle BreechOpenTimer;

};


