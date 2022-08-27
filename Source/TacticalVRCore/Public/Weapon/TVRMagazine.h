// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Grippables/GrippableStaticMeshActor.h"
#include "Interfaces/TVRHandSocketInterface.h"
#include "TVRMagazine.generated.h"


/**
 * Gripable Magazine Actor base class. Only children shall be spawned.
 */
UCLASS(Abstract)
class TACTICALVRCORE_API ATVRMagazine : public AGrippableStaticMeshActor, public ITVRHandSocketInterface
{
    GENERATED_BODY()

	UPROPERTY(Category="Gun", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	class UStaticMeshComponent* FollowerMesh;
	
	UPROPERTY(Category="Gun", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	class UInstancedStaticMeshComponent* RoundsInstances;
	
	/** Collider that detects if the magazine is touching the magwell properly */
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	UBoxComponent* MagazineCollider;
	
public:
    ATVRMagazine(const FObjectInitializer& OI);

    virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;

    virtual void ClosestGripSlotInRange_Implementation(
        FVector WorldLocation,
        bool bSecondarySlot,
        bool& bHadSlotInRange,
        FTransform& SlotWorldTransform,
        FName& SlotName,
        UGripMotionControllerComponent* CallingController = nullptr,
        FName OverridePrefix = NAME_None
    ) override;

	virtual bool DenyGripping_Implementation(UGripMotionControllerComponent* GripInitiator = nullptr) override;

	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed) override;

	virtual bool SimulateOnDrop_Implementation() override;

	virtual class UHandSocketComponent* GetHandSocket_Implementation(FName SlotName) const override;

	UFUNCTION(Category = "Magazine", BlueprintCallable, BlueprintNativeEvent)
	void GetFollowerLocationAndRotation(FVector& OutVector, FRotator& OutRotator) const;
	virtual void GetFollowerLocationAndRotation_Implementation(FVector& OutVector, FRotator& OutRotator) const;

	/**
     * Override so that the Primary Grip Type changes to prevent sudden detachment when the magazine is attached to a weapon
     */
    virtual EGripCollisionType GetPrimaryGripType_Implementation(bool bIsSlot) override;

    /**
     * Returns the transform of the Grip slot in world space. This still needs to be transformed to a relative grip
     * transform and compensated by the controller profile.
     * @param HandType Controller Hand Type so that we can give a different transform for left and right hand.
     */
    UFUNCTION(Category = "Magazine", BlueprintCallable, BlueprintNativeEvent)
    FTransform GetGripSlotTransform(UGripMotionControllerComponent* Hand) const;
    virtual FTransform GetGripSlotTransform_Implementation(UGripMotionControllerComponent* Hand) const;

	/** Initializes the Magazine */
	UFUNCTION(Category= "Magazine", BlueprintCallable)
	virtual void InitMagazine();

	/** Changes the collision profile for all magazine meshes, except helpers */
	UFUNCTION(Category= "Magazine", BlueprintCallable)
	virtual void SetCollisionProfile(FName NewProfile);
	
    /**
     * Is called when the magazine is fully ejected and returns to being a simulated physics body.
     * At this point there will be no connection to the weapon.
     * @param AngularVelocity Angular Velocity of the parent during the release
     * @param LinearVelocity Linear Velocity during the release
     */
    virtual void OnMagFullyEjected(const FVector& AngularVelocity, const FVector& LinearVelocity);

    /**
     * Tries to attach the magazine to a weapon
     * @param AttachComponent Component of the weapon to attach the magazine to
     * @param MagWell Corresponding Magwell
     * @param AttachTransform Transform that the magazines Attach Origin shall be attached to
     */
    virtual bool TryAttachToWeapon(USceneComponent* AttachComponent,
		class UTVRMagWellComponent* MagWell,
        const FTransform& AttachTransform);

    /**
     * Moves the Magazine so that the Transform of the Attach Origin Component aligns with the argument.
     * This way we are independent from the Meshes Transform
     * @param NewTransform Transform in World Space to align the Attach Origin to.
     */
    UFUNCTION(Category = "Magazine", BlueprintCallable)
    virtual void SetMagazineOriginToTransform(const FTransform& NewTransform);

    /**
     * @returns True if the magazine is inserted into an weapon
     */
    UFUNCTION(Category = "Magazine", BlueprintCallable)
    virtual bool IsInserted() const;

    /**
     * @returns True if the magazine is out of ammo
     */
	UFUNCTION(Category="Magazine", BlueprintCallable)
    bool IsEmpty() const;

    /**
     * Tries to consume ammo from the magazine
     * @returns true if ammo was successfully consumed (magazine is not empty)
     */
    virtual bool TryConsumeAmmo();

    /**
     * @returns Pointer to the Attach Origin Scene Component
     */
    virtual USceneComponent* GetAttachOrigin() const {return AttachOrigin;}

    /**
     * ReInitializes the Grip if needed
     */
    virtual void ReInitGrip();

    /**
     * Sets the current ammo to 
     */
    UFUNCTION(Category = "Magazine", BlueprintCallable)
    virtual void SetAmmo(int32 NewAmmo);

	int32 GetAmmo() const {return CurrentAmmo;}

    /**
     * Blueprint Event that is fired whenever the amount of ammo in the magazine has changed.
     */
    UFUNCTION(Category = "Magazine", BlueprintImplementableEvent)
    void BP_OnAmmoChanged();

    /** Percentage of how far the magazine is inserted */
    UPROPERTY(Category = "Magazine", BlueprintReadOnly)
    float MagInsertPercentage;

	/**
	 * @returns True if the mag release is pressed by the hand holding the magazine
	 */
	virtual bool IsMagReleasePressed() const;
	
	/** Event triggered by character once the player presses the mag release button */
	virtual void OnMagReleasePressed();
	
	/** Event triggered by character once the player releases the mag release button */
	virtual void OnMagReleaseReleased();

	/** 
	 * @returns the component that holds the rounds. Can also return null in case this component is non existent
	 */
	UFUNCTION(Category = "Magazine", BlueprintNativeEvent)
	UInstancedStaticMeshComponent* GetRoundsComponent() const;
	virtual UInstancedStaticMeshComponent* GetRoundsComponent_Implementation() const {return RoundsInstances;}
	
	/** 
	* @returns the follower mesh component. Is null in case there is no follower.
	*/
	UFUNCTION(Category = "Magazine", BlueprintNativeEvent)
    UStaticMeshComponent* GetFollowerComponent() const;
	virtual UStaticMeshComponent* GetFollowerComponent_Implementation() const {return FollowerMesh;}
	
	/** 
	* @returns the spring component. Can also return null in case there is no visible spring.
	*/
	UFUNCTION(Category = "Magazine", BlueprintNativeEvent)
    USkeletalMeshComponent* GetSpringComponent() const;
	virtual USkeletalMeshComponent* GetSpringComponent_Implementation() const {return nullptr;}

	/**
	 * Calculates the transform of a round at the specified index
	 * @param Index Index of the round, which transform we want to obtain
	 * @return The calculated transform of the round.
	 */
	UFUNCTION(Category = "Magazine", BlueprintNativeEvent, BlueprintCallable)
	FTransform GetRoundTransform(int32 Index) const;
	virtual FTransform GetRoundTransform_Implementation(int32 Index) const;

	/**
	 * Updates the transform of the follower
	 */
	UFUNCTION(Category = "Magazine", BlueprintNativeEvent, BlueprintCallable)
	void UpdateFollowerLocation();
	virtual void UpdateFollowerLocation_Implementation();

	/**
	 * Updates the number of round instances and their transforms.
	 */
	UFUNCTION(Category = "Magazine", BlueprintNativeEvent, BlueprintCallable)
	void UpdateRoundInstances();
	virtual void UpdateRoundInstances_Implementation();

	/**
	 * @returns the cartridge type of the magazine
	 */
	UFUNCTION(Category="Magazine", BlueprintCallable)
	TSubclassOf<class ATVRCartridge> GetCartridgeType() const {return CartridgeType;}

	/**
	 * @returns the mag well the magazine is inserted into or null if the magazine is free
	 */
	UFUNCTION(Category="Magazine", BlueprintCallable)
	UTVRMagWellComponent* GetAttachedMagWell()  const { return AttachedMagWell; }

	/**
	 * Returns a number between 0 adn 1 where 0 means the magazine is free and 1 means the magazine is fully inserted.
	 * @returns magazine insertion progress
	 */
	UFUNCTION(Category="Magazine", BlueprintCallable)
	float GetInsertProgress()  const { return MagInsertPercentage; }
	
	/** Radius of one round */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	float RoundRadius;

	/** Radius of the curve */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly, meta=(EditCond="CurveStartIdx >= 0"))
	float CurveRadius;

	/** Starting Index of the curve */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	int32 CurveStartIdx;

	/** Slope of the stack in uu/round*/
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	float StackSlope;

	/** scale of each round mesh instance */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	FVector RoundScale;

	/** Switch the left/right order for double stack magazines */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	bool bSwitchLROrder;

	/** Is this a double stack or single stack magazine */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	bool bDoubleStack;

	/** Slope of the follower springs morph target value 1/round */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	float FollowerMorphSlope;

	/** Bias of the follower springs morph target value */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	float FollowerMorphBias;

	/** Offset of the follower position of the calulated round transform */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	FVector FollowerOffset;

	/** Transform relative to the last round */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	FTransform FollowerRelativeTransform;

	/** NYI start index for the magazine taper, for tapered pistol mags */
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	int32 TaperStartIdx;
	
	UPROPERTY(Category = "Magazine", EditDefaultsOnly)
	TSubclassOf<class ATVRCartridge> CartridgeType;

	int32 GetDisplayAmmo() const;

	
	
protected:
	UPROPERTY()
	class UHandSocketComponent* HandSocket;
	
    /** Event Called on the magazine is fully ejected and is a physics body */
    UFUNCTION(Category = "Magazine", BlueprintImplementableEvent)
    void BP_OnMagFullyEjected();
	
	/** Is Magazine not Full on Init */
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditAnywhere)
	bool bNotFull;
	
    /** Capacity of Magazine */
    UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditDefaultsOnly, meta=(ClampMin=1, ClampMax=200))
    int32 AmmoCapacity;

    /**
     * Current remaining rounds in magazine. Do not access directly, unless you are sure.
     * It is recommended to use the SetAmmo function as it will also fire events for BP, so that
     * the visuals can be changed (remaining bullets, the follower, etc.) 
     */
    UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditAnywhere, meta=(EditCondition ="bNotFull", ClampMin=0, ClampMax=200))
    int32 CurrentAmmo;

	/** Limit of the displayed Ammo. A Value of 0 or below will mean no limit. */
	UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditAnywhere, meta=(ClampMin=-1, ClampMax=200))
	int32 LimitDisplayAmmo;
	
    /** Is Magazine attached to a weapon? DEPRECATED */
    bool bIsAttached;
	
	UPROPERTY(Category="Magazine", BlueprintReadOnly)
	class UTVRMagWellComponent* AttachedMagWell;

    /** Scene Component that marks the Slot the hand is gripping */
    UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditAnywhere)
    USceneComponent* GripSlot;

    /** Scene Component that marks the Point that should be aligned with the weapons magazine attach point */
    UPROPERTY(Category = "Magazine", BlueprintReadOnly, EditAnywhere)
    USceneComponent* AttachOrigin;


	/** Alternatve Mag Release method, by pressing the mag release button while holding the magazine */
	UPROPERTY(Category="Magazine", BlueprintReadOnly)
	bool bIsMagReleasePressed;

	/** Array of mesh components of this magazine. Used when the collision of the entire magazine needs to be modified */
	UPROPERTY()
	TArray<UStaticMeshComponent*> MagazineMeshes;
};
