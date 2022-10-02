// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Weapon/TVRMagazine.h"
#include "Interfaces/TVRMagazineInterface.h"
#include "TVRMagazineWell.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleMagazineWellDelegate);

/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVRMagazineWell : public UChildActorComponent, public ITVRMagazineInterface
{
	GENERATED_BODY()

	UPROPERTY()
	class UBoxComponent* CollisionBox;

	UPROPERTY()
	class UAudioComponent* MagAudioComp;
	
public:
	UTVRMagazineWell(const FObjectInitializer& OI);

	FVector GetEjectLocation() const { return GetComponentTransform().TransformPosition(EjectRelativeLoc);}
	const FVector& GetEjectRelativeLocation() const { return EjectRelativeLoc;}
	FVector GetCollisionExtent() const { return CollisionExtent;}
	FTransform GetCollisionTransform() const { return CollisionRelativeTransform * GetComponentTransform(); }

	UPROPERTY(Category="Magazine Well", EditDefaultsOnly)
	FVector CollisionExtent;
	UPROPERTY(Category="Magazine Well", EditDefaultsOnly)
	FTransform CollisionRelativeTransform;
	
	UPROPERTY(Category="Magazine Well", EditDefaultsOnly)
	FVector EjectRelativeLoc;
		
protected:
	virtual void CreateChildActor() override;

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	
	
public:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	class ATVRGunBase* GetGunOwner() const;

	void InitChildMag();
	
	// ================================================
	// Start MagazineComponentInterface
	// ================================================

	/**
	 * Changes the collision profile of all relevant meshes (anything that isn't a helper but a physical collider)
	 * @param NewProfile Name of the new collision profile
	 */
	virtual void SetMagazineCollisionProfile_Implementation(FName NewProfile) override;

	/**
	 * @returns True if the conditions are met from the magazine side for the bolt to be locked.
	 */
	virtual bool CanBoltLock_Implementation() const override;
	
	/**
	 * @returns True if the attached magazine is empty.
	 */
	virtual bool IsEmpty_Implementation() const override;

	/**
	 * @returns number between 0 and 1, where 1 means magazine is fully inserted
	 */
	virtual float GetAmmoInsertProgress_Implementation() const override;

	virtual void OnMagReleasePressed_Implementation(bool bAlternatePress = false) override;
	virtual void OnMagReleaseReleased_Implementation(bool bAlternatePress = false) override;

	virtual void OnOwnerGripReleased_Implementation(ATVRCharacter* OwningChar, class UGripMotionControllerComponent* ReleasingHand) override;
	/**
	 * @returns True if Ammo can be fed from the magazine to the weapon
	 */
	virtual bool CanFeedAmmo_Implementation() const override;

	virtual TSubclassOf<class ATVRCartridge> TryFeedAmmo_Implementation() override;
	
	// ================================================
	// End MagazineComponentInterface
	// ================================================

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
	

    /**
     * Handles the Mag Drop Logic during Component Tick. DO NOT CALL OUTSIDE OF TickComponent()
     * @param DeltaSeconds Delta Time between the last Frame in seconds
     */
	virtual void HandleMagDrop(float DeltaSeconds);

	virtual void HandleMagInsert(float DeltaSeconds);
	virtual void HandleMagFall(float DeltaSeconds);

    /**
     * Event Called when the mag is fully ejected and not constrained by the Magwell anymore.
     */
	virtual void OnMagFullyEjected();

	virtual void OnMagDestroyed();

	virtual void OnMagFullyInserted();
	
	/**
	 * @returns True if the magazine is fully ejected, i.e. left the magwell and is not constrained by it anymore.
	 */
	virtual bool ShouldEjectMag() const;
    
    virtual bool ShouldInsertMag() const;

	/**
	 * @returns True if a magazine can be inserted (on overlap)
	 */
	virtual bool CanInsertMag() const;

	/**
	 * Check if a magazine is currently in the Magwell.
	 */
	virtual bool HasMagazine() const {return CurrentMagazine != nullptr;}

    /**
    * Check if a magazine is currently in the Magwell.
    */
    virtual bool HasFullyInsertedMagazine() const { return HasMagazine() && !bIsMagFree; }
    
	/**
	 * Tries to Release the Mag.
	 * @returns True if the magazine was successfully released
	 */
	virtual bool TryReleaseMag();
	
	/**
	 * Checks a given class is one of the allowed Magazine Types.
	 * @param TestClass Class to test
	 * @returns True of the class is an allowed magazine type
	 */
	virtual bool IsAllowedMagType(UClass* TestClass) const {return AllowedMagazines.Find(TestClass) != INDEX_NONE;}

	/**
	 * Returns the currently inserted magazine. If no magazine was inserted it will return nullptr.
	 * There is a possibility for this function to return a unsafe pointer. Check HasMagazine
	 * in to also consider validity.
	 * @returns Pointer to the current Magazine
	 */
    UFUNCTION(Category = "Magazine", BlueprintCallable)
	virtual class ATVRMagazine* GetCurrentMagazine() const {return CurrentMagazine;}


    virtual bool IsMagReleasePressed_Implementation() const override;

	virtual float GetSplineTransform(const FVector& inLoc, FTransform& outTransform) const;

	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FSimpleMagazineWellDelegate EventOnMagazineFullyDropped;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FSimpleMagazineWellDelegate EventOnMagazineFullyInserted;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FSimpleMagazineWellDelegate EventOnMagazineStartDrop;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FSimpleMagazineWellDelegate EventOnMagazineStartInsert;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FSimpleMagazineWellDelegate EventOnMagReleasePressed;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FSimpleMagazineWellDelegate EventOnMagReleaseReleased;

	bool HasMagRelease() const {return bHasMagRelease;}

	TSubclassOf<class ATVRMagazine> DefaultMagazineClass;

	virtual void GetAllowedCatridges_Implementation(TArray<TSubclassOf<class ATVRCartridge>>& OutCartridges) const override;
	
protected:
	virtual void StartInsertMagazine(class ATVRMagazine* MagToInsert);

	UFUNCTION()
	virtual void StopIgnoreMag();
	
    /** Reference to the current magazine. Check Validitiy if you want to be safe. */
	UPROPERTY()
	class ATVRMagazine* CurrentMagazine;

    /** Returns true of the mag is being dropped right now */
	bool bIsMagFree;

    bool bMagReleasePressed;

	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	bool bHasMagRelease;

    /** Velocity Vector of Attached Magazine */
    FVector MagVelocity;
	
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	float MagEjectAcceleration;

	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	bool bUseCurve;
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly, meta=(EditCondition=bUseCurve))
	FRuntimeFloatCurve MagRoll;
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly, meta=(EditCondition=bUseCurve))
	FRuntimeFloatCurve MagPitch;
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly, meta=(EditCondition=bUseCurve))
	FRuntimeFloatCurve MagYaw;
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	class USoundBase* MagazineSound;

	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	bool bNeedsToBeReleasedByHand;
	bool bWasReleasedByHand;
	
	/** Allowed Magazines that can be inserted into this magwell */
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	TArray<TSubclassOf<ATVRMagazine> > AllowedMagazines;

	float DetachProgress;
	bool bCanDetach;
	UPROPERTY()
	class ATVRMagazine* IgnoreMagazine;
	FTimerHandle IgnoreMagTimer;
};
