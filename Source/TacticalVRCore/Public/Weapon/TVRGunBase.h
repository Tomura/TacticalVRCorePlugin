// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/TVRHandSocketInterface.h"
#include "Grippables/GrippableStaticMeshActor.h"

#include "TVRGunBase.generated.h"


UENUM(BlueprintType)
enum class ETVRGunType : uint8
{
	Primary,
	Sidearm
};

UENUM(BlueprintType)
enum class ETVRGunClass : uint8
{
	Pistol,
    Rifle
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunSecondaryUsedDelegate);

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API ATVRGunBase : public AGrippableStaticMeshActor, public ITVRHandSocketInterface
{
	GENERATED_BODY()

	friend class ATVRGunWithChild;

	/** Primary Trigger Component for shooting */
	UPROPERTY(Category="Gun", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	class UTVRTriggerComponent* TriggerComponent;
	
	UPROPERTY(Category="Gun", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	class UTVRGunFireComponent* FiringComponent;
	
	UPROPERTY(Category="Gun", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	class UAudioComponent* GunManipulationAudioComponent;
	
	UPROPERTY(Category="Gun", BlueprintReadOnly, EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	class USkeletalMeshComponent* MovablePartsMesh;

	
	UPROPERTY()
	class UAudioComponent* SelectorAudio;
	
public:

	static FName PrimarySlotName;
	static FName SecondarySlotName;

protected:
	
	/** Current Secondary Grip Controller */
	UPROPERTY()
	UGripMotionControllerComponent* SecondaryController;
	
	/** Current Secondary Grip Info */
	FBPActorGripInformation SecondaryGripInfo;
	
	UPROPERTY()
	TArray<class UStaticMeshComponent*> GunMeshes;
	TArray<class UTVRAttachmentPoint*> AttachmentPoints;

	UPROPERTY()
	class UStaticMeshComponent* BoltMesh;
	FVector BoltMeshInitialRelativeLocation;

	UPROPERTY()
	class UTVREjectionPort* EjectionPort;

	UPROPERTY()
	USceneComponent* ChargingHandleInterface;
	
	UPROPERTY(Category="Gun|Grip|OneHand", EditDefaultsOnly)
	float OneHandStiffness;
	UPROPERTY(Category="Gun|Grip|OneHand", EditDefaultsOnly)
	float OneHandDamping;
	UPROPERTY(Category="Gun|Grip|OneHand", EditDefaultsOnly)
	float OneHandAngularStiffness;
	UPROPERTY(Category="Gun|Grip|OneHand", EditDefaultsOnly)
	float OneHandAngularDamping;
	UPROPERTY(Category="Gun|Grip|TwoHand", EditDefaultsOnly)
	float TwoHandStiffness;
	UPROPERTY(Category="Gun|Grip|TwoHand", EditDefaultsOnly)
	float TwoHandDamping;
	UPROPERTY(Category="Gun|Grip|TwoHand", EditDefaultsOnly)
	float TwoHandAngularStiffness;
	UPROPERTY(Category="Gun|Grip|TwoHand", EditDefaultsOnly)
	float TwoHandAngularDamping;
	
public:
	ATVRGunBase(const FObjectInitializer& OI);

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void InitAttachmentPoints();
	
	/**
	 * Usually called when the actor is finished spawning
	 */
	virtual void BeginPlay() override;

	/**
	 * Called every tick
	 * @param DeltaSeconds Time between the last frame in seconds
	 */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Called before the actor is destroyed
	 */
	virtual void BeginDestroy() override;

	virtual void TickBolt(float DeltaSeconds);
	virtual void TickHammer(float DeltaSeconds);

	virtual void CheckBoltEvents(float PreviousBoltProgress);
	
	UFUNCTION()
	virtual void OnEndFiringCycle();
	
	UFUNCTION(Category="Gun", BlueprintCallable)
	class UTVRTriggerComponent* GetTriggerComponent() const {return TriggerComponent;}
	
	UFUNCTION(Category="Gun", BlueprintCallable)
	class UTVRGunFireComponent* GetFiringComponent() const {return FiringComponent;}
	
	// VRGripInterface
	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo) override;
	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo, bool bWasSocketed) override;
	virtual void OnSecondaryGrip_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* SecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;
	virtual void OnSecondaryGripRelease_Implementation(UGripMotionControllerComponent* GripOwningController, USceneComponent* ReleasingSecondaryGripComponent, const FBPActorGripInformation& GripInformation) override;
	virtual void OnSecondaryUsed_Implementation() override;
	virtual void OnEndSecondaryUsed_Implementation() override;
	
	FOnGunSecondaryUsedDelegate EventOnSecondaryUsed;
	FOnGunSecondaryUsedDelegate EventOnSecondaryEndUsed;
	FVROnGripSignature EventOnSecondaryGripped;
	FVROnGripSignature EventOnSecondaryDropped;
	FVROnGripSignature EventOnGripped;
	FVROnDropSignature EventOnDropped;
	
	virtual void ClosestGripSlotInRange_Implementation(FVector WorldLocation, bool bSecondarySlot,  bool & bHadSlotInRange, FTransform & SlotWorldTransform, FName & SlotName, UGripMotionControllerComponent * CallingController = nullptr, FName OverridePrefix = NAME_None) override;

	virtual bool GetSecondarySlot(FVector WorldLocation, FTransform & OutTransform, FName & OutSlotName, UGripMotionControllerComponent * CallingController) const;
	
	virtual USceneComponent* GetPrimaryGripSlot() const {return GetPrimaryHandSocket();}

	virtual bool RequestsSocketing_Implementation(USceneComponent*& ParentToSocketTo, FName& OptionalSocketName, FTransform_NetQuantize& RelativeTransform) override;

	virtual bool DenyGripping_Implementation(UGripMotionControllerComponent* GripInitiator = nullptr) override;

	// Handsocket
	class UHandSocketComponent* GetHandSocket_Implementation(FName SocketName) const;
	
	// Methods
	class ACharacter* GetCharacterOwner() const;
	class ATVRCharacter* GetVRCharacterOwner() const;
	class AController* GetOwnerController() const;
    bool IsOwnerLocalPlayerController() const;
	bool IsHeldByParentGun() const;
	class ATVRGunWithChild* GetParentGun() const;


	UFUNCTION(Category = "Gun", BlueprintImplementableEvent, BlueprintCallable)
	void FoldSights(bool bFold);
	
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void HideRearSight(bool bFold);
	
	UFUNCTION(Category = "Gun|Collision", BlueprintCallable, BlueprintNativeEvent)
	void SetCollisionProfile(FName NewProfile);
	virtual void SetCollisionProfile_Implementation(FName NewProfile);
	
	UFUNCTION(Category = "Gun|Collision", BlueprintImplementableEvent, BlueprintCallable)
	void GetWeaponMeshes(TArray<UStaticMeshComponent*>& Meshes) const;

	
	virtual void OnFullyDropped(class ATVRCharacter* GrippingCharacter, class UGripMotionControllerComponent* GrippingHand);
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void BPOnFullyDropped();
	
    /**
     * Re Initializes the secondary grip, when the primary slot was gripped and the secondary grip was attach as primary
     * @param GrippingHand Hand that is gripping the weapon
     * @param GripInfo Grip Information
     */
	virtual void ReInitSecondary(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo);

    /**
     * Handles the swap of the primary hand, in case the primary hand is released, but a secondary grip is attached.
     * @param GripInfo Grip Information (actually release information)
     */
	virtual bool HandleHandSwap(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo);

    /**
     * Returns the component that marks the secondary slot
     * @returns Secondary slot component as Scene Component
     */
	UFUNCTION(Category = "Gripping", BlueprintNativeEvent, BlueprintCallable)
	USceneComponent* GetSecondarySlotComponent() const;
	virtual USceneComponent* GetSecondarySlotComponent_Implementation() const;

    /**
     * Called to start fire (trigger is pressed)
     * Will simulate an empty gun if necessary
     */
	UFUNCTION(Category = "Firing", BlueprintCallable)
	virtual void OnStartFire();
	
    /**
     * Called to stop fire (trigger is released)
     */
	UFUNCTION(Category = "Firing", BlueprintCallable)
	virtual void OnStopFire();    

	
    UFUNCTION(Category = "Firing", BlueprintCallable)
    virtual void AddRecoil();

	
	UFUNCTION(Category = "Firing", BlueprintCallable)
	virtual void AddCustomRecoil(const FTransform& PointOfAttack, const FVector& RecoilImpulse, const FVector& AngularRecoilImpulse = FVector::ZeroVector);

	UFUNCTION()
	virtual void OnFire();
	UFUNCTION()
	virtual void OnEmpty();

	UFUNCTION()
	virtual void OnCartridgeSpent();
	
	virtual bool CanStartFire() const;
	
	UFUNCTION(Category="Gun", BlueprintCallable)
	ETVRGunType GetGunType() const {return GunType;} 

	UFUNCTION(Category="Gun", BlueprintCallable)
	USceneComponent* GetChargingHandleInterface() const;
	
    bool HasLastRoundBoltHoldOpen() const;
	
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void BP_OnBoltReleasedByChargingHandle();
	
	virtual void OnBoltReleased();
	
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void BP_OnBoltReleased();

	
	virtual void OnMagReleasePressedFromPrimary();
	virtual void OnMagReleaseReleasedFromPrimary();

	
	UFUNCTION(Category = "Gun", BlueprintCallable)
	virtual void OnMagReleasePressed();
	UFUNCTION(Category = "Gun", BlueprintCallable)
	virtual void OnMagReleaseReleased();
	
	/**
	 * Called in order to operate the mag release by seconday hand in case you are grabbing a
	 * under barrel weapon (Grenade Launcher, Shotgun)
	 * @returns true if the action was handled
	 */
	virtual bool OnSecondaryMagReleasePressed();
	/**
	* Called in order to operate the mag release by seconday hand in case you are grabbing a
	* under barrel weapon (Grenade Launcher, Shotgun)
	* @returns true if the action was handled
	*/
	virtual bool OnSecondaryMagReleaseReleased();
	
	/**
	* Called in order to operate the bolt release by seconday hand in case you are grabbing a
	* under barrel weapon (Grenade Launcher, Shotgun)
	* @returns true if the action was handled
	*/
	virtual bool OnSecondaryBoltReleasePressed();

	/**
	* Called in order to operate the bolt release by seconday hand in case you are grabbing a
	* under barrel weapon (Grenade Launcher, Shotgun)
	* @returns true if the action was handled
	*/
	virtual bool OnSecondaryBoltReleaseReleased();
	

	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void BP_OnMagReleasePressed();
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void BP_OnMagReleaseReleased();
	
	/**
	 * 
	 */
	UFUNCTION(Category = "Gun", BlueprintCallable, BlueprintNativeEvent)
	void InitMagInterface();
	virtual void InitMagInterface_Implementation();

	void InitEjectionPort();
	
	void InitChargingHandle();
	
	UFUNCTION(Category = "Gun", BlueprintCallable)
	class UTVRMagazineCompInterface* GetMagInterface() const {return MagInterface;}

	virtual void EjectRound(bool bSpent=true);
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void OnEjectRound(bool bSpent, TSubclassOf<class ATVRCartridge> CartridgeType);
	
	virtual bool TryChamberNewRound();
	UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
	void OnChamberRound();

    bool CanUseBoltRelease() const;
    bool CanUseMagRelease() const;

	void OnBoltReleasePressedFromPrimary();	
	void OnBoltReleaseReleasedFromPrimary();
	
	UFUNCTION(Category = "Gun", BlueprintCallable)
    void OnBoltReleasePressed();
	
	UFUNCTION(Category = "Gun", BlueprintCallable)
    void OnBoltReleaseReleased();
	
    UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
    void BP_OnBoltReleasePressed();
    UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
    void BP_OnBoltReleaseReleased();

    /**
     * Called to cycle firing mode.
     */
    virtual void OnCycleFiringMode();
    UFUNCTION(Category = "Gun", BlueprintImplementableEvent)
    void BP_OnCycleFiringMode();

    UFUNCTION(Category = "Gun", BlueprintCallable)
    bool IsGrippedAtPrimaryGrip() const;
	
    UFUNCTION(Category = "Gun", BlueprintCallable)
	float GetBoltProgress() const {return BoltProgress;}
	UFUNCTION(Category = "Gun", BlueprintCallable)
	float GetHammerProgress() const {return HammerProgress;}
    
    UFUNCTION(Category= "Gun", BlueprintCallable)
    bool IsBoltReleasePressed() const {return bBoltReleasePressed;}
    UFUNCTION(Category= "Gun", BlueprintCallable)
    bool IsMagReleasePressed() const;

    bool IsBoltLocked() const {return bIsBoltLocked;}

	const TArray<UTVRAttachmentPoint*>& GetAttachmentPoints() const { return AttachmentPoints; }

	template<class T> 
	T* GetAttachmentPoint() const
	{
    	TArray<T*> AttachPoints;
    	GetComponents<T>(AttachPoints);
    	if(AttachPoints.Num() > 0)
    	{
    		return AttachPoints[0];
    	}
    	return nullptr;
    }

	template<class T> 
	T* GetAttachment() const
    {
	    TArray<AActor*> ChildrenActors;
    	GetAllChildActors(ChildrenActors);
    	for(const auto TestChild: ChildrenActors)
    	{
    		if(auto AttachmentActor = Cast<T>(TestChild))
    		{
    			return AttachmentActor;
    		}
    	}
    	return nullptr;
    }

	UFUNCTION(Category="Gun", BlueprintImplementableEvent)
	void OnBarrelChanged(TSubclassOf<class AWPNA_Barrel> NewBarrel);

	UFUNCTION(Category="Gun", BlueprintNativeEvent)
	void ToggleLaser(UGripMotionControllerComponent* UsingHand);
	virtual void ToggleLaser_Implementation(UGripMotionControllerComponent* UsingHand);
	UFUNCTION(Category="Gun", BlueprintNativeEvent)
	void ToggleLight(UGripMotionControllerComponent* UsingHand);
	virtual void ToggleLight_Implementation(UGripMotionControllerComponent* UsingHand);
	
	UFUNCTION(Category = "Gun", BlueprintNativeEvent)
    void GetRecoilPointOfAttack(FTransform& OutTransform) const;
	virtual void GetRecoilPointOfAttack_Implementation(FTransform& OutTransform) const;

	bool HasMagReleaseOnPrimaryGrip() const { return bHasMagReleaseOnPrimaryGrip;}
	bool HasBoltReleaseOnPrimaryGrip() const { return bHasBoltReleaseNearPrimaryGrip;}

	UFUNCTION(Category= "Gun", BlueprintNativeEvent)
	bool GetPrimaryHandTransform(FTransform& OutTransform, EControllerHand HandType) const;
	virtual bool GetPrimaryHandTransform_Implementation(FTransform& OutTransform, EControllerHand HandType) const {return false;}

	UGripMotionControllerComponent* GetSecondaryController() const;
	const FBPActorGripInformation* GetSecondaryGripInfo() const;
	
	/**
	 * @returns the Primary hand socket of this gun
	 */
	UFUNCTION(Category= "Gun", BlueprintNativeEvent)
	class UHandSocketComponent* GetPrimaryHandSocket() const;
	virtual class UHandSocketComponent* GetPrimaryHandSocket_Implementation() const { return nullptr; }
	
	/**
	* @returns the Secondary hand socket of this gun
	*/
	UFUNCTION(Category= "Gun", BlueprintNativeEvent)
	class UHandSocketComponent* GetSecondaryHandSocket() const;
	virtual class UHandSocketComponent* GetSecondaryHandSocket_Implementation() const { return nullptr; }
	

	UPROPERTY(Category= "Gun", EditAnywhere)
	TSubclassOf<class ATVRMagazine> SpawnMagazineOverride;

	/**
	 * @returns the movable parts mesh. Returns nullptr if it is not available.
	 */
	UFUNCTION(Category= "Gun", BlueprintCallable)
	USkeletalMeshComponent* GetMovablePartsMesh() const {return MovablePartsMesh;}
	

	void SetConstraintToTwoHanded();
	void SetConstraintToOneHanded();

	UFUNCTION(Category="Gun", BlueprintCallable)
	void OnPhysicsHit(AActor* HitActor, AActor* OtherActor, FVector HitVelocity, const FHitResult& Hit);

	UFUNCTION(Category="Gun", BlueprintCallable, BlueprintNativeEvent)
	bool ShouldLockBolt() const;
	virtual bool ShouldLockBolt_Implementation() const {return false;};

	/** little hack to force the blueprint to recompile */
	UPROPERTY(Category="Weapon Attachments", EditAnywhere, AdvancedDisplay)
	bool bForceRecompile;

	
	UFUNCTION(Category="Gun", BlueprintCallable)
	void SetColorVariant(uint8 newVariant);
	
	UFUNCTION(Category="Gun", BlueprintImplementableEvent)
	void OnColorVariantChanged(uint8 newVariant);	
	
protected:

	UFUNCTION(Category="Gun", BlueprintCallable)
	void SetBoltMesh(class UStaticMeshComponent* NewMesh);

	UFUNCTION()
	void OnOpenDustCover();

	void OnBoltClosed();
	UFUNCTION(Category="Gun", BlueprintImplementableEvent)
	void OnSimulateBoltClosed();

	UFUNCTION(Category="Gun", BlueprintCallable)
	void LockBoltIfNecessary();
	
	UFUNCTION(Category="Gun", BlueprintCallable)
	void UnlockBoltIfNecessary();
	
	void LockBolt();
	void UnlockBolt();
	
	virtual void CollectWeaponMeshes();
	
	/** Weak reference to primary hand */
	TWeakObjectPtr<class UGripMotionControllerComponent> PrimaryHand;
	
	UPROPERTY()
    class UStaticMeshComponent* LoadedBullet;
    
	UPROPERTY(Category="Gripping", BlueprintReadOnly, Instanced)
	UGS_GunTools* GripScript;
	
    bool bSkipHandSwap;
    
	bool bIsSocketed;
	bool bIsFiring;
	bool bHasRoundInChamber;
	TSubclassOf<class ATVRCartridge> RoundInChamber;

    UPROPERTY(Category = "Gun", BlueprintReadOnly)
    float BoltProgress;
	
	UPROPERTY(Category = "Gun", BlueprintReadOnly)
	float BoltMovePct;
    
    UPROPERTY(Category = "Gun", BlueprintReadOnly)
	bool bIsBoltLocked;
	
	UPROPERTY(Category = "Gun", BlueprintReadOnly)
	bool bBoltReleasePressed;	

	UPROPERTY(Category = "Gun|Sound", EditDefaultsOnly)
	USoundBase* SelectorSound;
	
    UPROPERTY(Category = "Gun|Features", BlueprintReadOnly, EditDefaultsOnly)
    uint8 bHasLastRoundBoltHoldOpen : 1;
	
	UPROPERTY(Category = "Gun|Features", BlueprintReadOnly, EditDefaultsOnly)
	uint8 bHasMagReleaseOnPrimaryGrip : 1;

	UPROPERTY(Category = "Gun|Features", BlueprintReadOnly, EditDefaultsOnly)
	uint8 bHasBoltReleaseNearPrimaryGrip : 1;   
	
	/**
	 * Whether our gun cycless automatically. Leave on true for Automatic or Halfautomatic weapons
	 * Set to false for Pump-Action or Bolt-Action, or anything that requires manual cycling
	 */
	UPROPERTY(Category = "Gun|Features", BlueprintReadOnly, EditDefaultsOnly)
	uint8 bDoesCycle : 1;
 
    /** Type of gun. Used mainly for sorting. */
	UPROPERTY(Category = "Gun", EditDefaultsOnly)
	ETVRGunType GunType;
	
    /** best used for pistols or other single hand weapons */
    UPROPERTY(Category = "Gun", EditDefaultsOnly)
    bool bHandSwapToPrimaryGripSlot;
	
	UPROPERTY(Category = "Gun", EditDefaultsOnly)
	class UTVRMagazineCompInterface* MagInterface;
	
	UPROPERTY(Category = "Gun|Recoil", BlueprintReadOnly, EditDefaultsOnly)
	FVector RecoilImpulse;
	UPROPERTY(Category = "Gun|Recoil", BlueprintReadOnly, EditDefaultsOnly)
	FVector RecoilAngularImpulse;
	
	UPROPERTY(Category = "Gun|Recoil", BlueprintReadOnly, EditDefaultsOnly)
	float RecoilReductionTwoHand;
	
	UPROPERTY(Category = "Gun", BlueprintReadOnly, EditDefaultsOnly)
	class UHapticFeedbackEffect_Base* HapticFeedback_Fire;

	/** Random Stream for Recoil. Should be synced on all clients for a synced random recoil stream */
    FRandomStream RecoilStream;
	
	/**
	 * Secondary Hand that is currently attached as primary,
	 * but needs to be reattached as secondary once the primary slot is gripped again
	 */
	TWeakObjectPtr<class UGripMotionControllerComponent> SavedSecondaryHand;
	
	/** Relative Transform of the secondary hand, used when re attaching the hand as a secondary. */
	FTransform SavedSecondaryHandRelTransform;
	
	/** Saved secondary grip slot name. */
	FName SavedSecondarySlotName;

	UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float BoltProgressOpenDustCover;
    UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float BoltProgressEjectRound;
    UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float BoltProgressFeedRound;
	UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly, meta=(ClampMin="0.0"))
	float BoltStiffness;

	float BoltProgressSpeed;
	UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly, BlueprintReadWrite, meta=(ClampMin="0.0"))
	float BoltStroke;

	float HammerProgress;
	bool bHammerLocked;
	UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly, meta=(ClampMin="0.0"))
	float BoltProgressHammerCocked;
	UPROPERTY(Category = "Gun|Bolt", EditDefaultsOnly)
	bool bHammerDoubleAction;
	
	UPROPERTY(Category = "Gun", EditDefaultsOnly)
	FText DisplayName;

	UPROPERTY(Category = "Gun", EditAnywhere)
	uint8 ColorVariant;
};


