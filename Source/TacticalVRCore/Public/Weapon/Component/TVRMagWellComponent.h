// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Component/TVRMagazineCompInterface.h"
#include "Components/SplineComponent.h"

#include "TVRMagWellComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineStartInsertDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineFullyInsertedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineFullyDroppedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagazineStartDropDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagReleasePressedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMagReleaseReleasedDelegate);
/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (TacticalVR))
class TACTICALVRCORE_API UTVRMagWellComponent : public UTVRMagazineCompInterface
{
	GENERATED_BODY()


	UPROPERTY()
	class UAudioComponent* MagAudioComp;
	
public:
	UTVRMagWellComponent(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

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

	virtual void OnMagReleasePressed(bool bAlternatePress = false) override;
	virtual void OnMagReleaseReleased(bool bAlternatePress = false) override;

	virtual void OnOwnerGripReleased(ATVRCharacter* OwningChar, class UGripMotionControllerComponent* ReleasingHand) override;
	/**
	 * @returns True if Ammo can be fed from the magazine to the weapon
	 */
	virtual bool CanFeedAmmo() const override;

	virtual TSubclassOf<class ATVRCartridge> TryFeedAmmo() override;
	
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
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;

    /**
     * Transforms the transform of a spline point into a transform of a magazine attach origin on this spline
     * @param InTransform Transform along the spline that needs to be transformed
     * @returns Transform to attach a magazine to.
     */
	FTransform TransformSplineToMagazineCoordinates(const FTransform& InTransform) const;

    /**
     * @returns a pointer to the magazine spline
     */
	USplineComponent* GetMagSpline() const;
	USplineComponent* FindMagSpline() const;

    /**
     * Repositions the magazine to the desired position
     */
	virtual void RepositionMagazine();

    /**
     * Handles the Mag Drop Logic during Component Tick. DO NOT CALL OUTSIDE OF TickComponent()
     * @param DeltaSeconds Delta Time between the last Frame in seconds
     */
	virtual void HandleMagDrop(float DeltaSeconds);

    /**
     * Event Called when the mag is fully ejected and not constrained by the Magwell anymore.
     */
	virtual void OnMagFullyEjected();

	virtual void OnMagDestroyed();

	/**
	 * @returns True if the magazine is fully ejected, i.e. left the magwell and is not constrained by it anymore.
	 */
	virtual bool ShouldEjectMag() const;


    virtual void OnMagFullyInserted();
    
    virtual bool ShouldInsertMag() const;

	/**
	 * @returns True if a magazine can be inserted (on overlap)
	 */
	virtual bool CanInsertMag() const;

	/**
	 * Check if a magazine is currently in the Magwell.
	 */
	virtual bool HasMagazine() const;

    /**
    * Check if a magazine is currently in the Magwell.
    */
    virtual bool HasFullyInsertedMagazine() const;
    
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
	virtual bool IsAllowedMagType(UClass* TestClass) const;

	/**
	 * Returns the currently inserted magazine. If no magazine was inserted it will return nullptr.
	 * There is a possibility for this function to return a unsafe pointer. Check HasMagazine
	 * in to also consider validity.
	 * @returns Pointer to the current Magazine
	 */
    UFUNCTION(Category = "Magazine", BlueprintCallable)
	virtual class ATVRMagazine* GetCurrentMagazine() const;


    virtual bool IsMagReleasePressed() const override;

	virtual void GetSplineTransform(const FVector& inLoc, FTransform& outTransform) const;
	virtual void GetSplineTransformAtTime(float inProgress, FTransform& outTransform) const;

	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FOnMagazineFullyDroppedDelegate EventOnMagazineFullyDropped;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FOnMagazineFullyInsertedDelegate EventOnMagazineFullyInserted;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FOnMagazineStartDropDelegate EventOnMagazineStartDrop;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FOnMagazineStartInsertDelegate EventOnMagazineStartInsert;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FOnMagReleasePressedDelegate EventOnMagReleasePressed;
	UPROPERTY(Category = "Magazine", BlueprintAssignable)
	FOnMagReleaseReleasedDelegate EventOnMagReleaseReleased;

	bool HasMagRelease() const {return bHasMagRelease;}

	UFUNCTION(Category="Magazine", BlueprintCallable)
	virtual class ATVRMagazine* SpawnMagazineAttached(TSubclassOf<ATVRMagazine> MagazineClass = nullptr);

	TSubclassOf<ATVRMagazine> DefaultMagazineClass;
	
protected:
	virtual void StartInsertMagazine(class ATVRMagazine* MagToInsert);
	
    /** Reference to the current magazine. Check Validitiy if you want to be safe. */
	UPROPERTY()
	class ATVRMagazine* CurrentMagazine;

    /**
     * Spline Component that marks the path from the Mag Attach Point to the end of the magwell.
     * The first point marks the mag attach point.
     * The last point marks the point at which the mag is not constrained anymore.
     * The transform of the spline shall be setup so that the tangents (X-axis) points outside of the mag well.
     * The Z axis of each point shall point forward.
     * During mag drop the magazine will be facing this way.
     */
	UPROPERTY()
	class USplineComponent* CachedMagSpline;

    /** Returns true of the mag is being dropped right now */
	bool bIsMagFree;

    bool bMagReleasePressed;

	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly)
	bool bHasMagRelease;

    /** Velocity Vector of Attached Magazine */
    FVector MagVelocity;

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
};
