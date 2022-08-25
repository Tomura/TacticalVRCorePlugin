// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "TVRTypes.h"
#include "TVRGraspingHand.h"
#include "VRCharacter.h"
#include "TVRCharacter.generated.h"

/** Turn Direction enum to improve code readability */
UENUM(BlueprintType)
enum class ETurnDirection : uint8 {
	Left,
	Right
};

/**
 * Struct that stores information about a object to be grabbed.
 * Mostly used to pass the data through the functions involved in
 * processing the grabbing logic, so that it is reduced to only one reference
 * instead of multiple references of variables.
 */
USTRUCT(BlueprintType)
struct FGrabObjectInfo
{
	GENERATED_BODY()
public: 
    FGrabObjectInfo() :
		ObjectToGrip(nullptr),
		bIsSecondaryGrip(false),
		bIsSlotGrip(false),
		GripTransform(FTransform_NetQuantize::Identity),
		GripBoneName(EName::NAME_None),
		SlotName(EName::NAME_None)
    { }

    /** Object that should be gripped. */
	UObject* ObjectToGrip;

    /** Is it a secondary grip? */
	bool bIsSecondaryGrip;

    /** Is it a slot grip? */
	bool bIsSlotGrip;

    /** Transform of the grip. */
	FTransform_NetQuantize GripTransform;	

    /** Gripped Bone Name */
    FName GripBoneName;

    /** Name of the Slot */
	FName SlotName;	
};


/**
 * Abstract base class for a Character. You should not be able to spawn an instance.
 */
UCLASS(Abstract)
class TACTICALVRCORE_API ATVRCharacter : public AVRCharacter
{
	GENERATED_BODY()
public:
	/**
	 * Constructor. Sets default values.
	 * Most of those will probably be overriden in a Blueprint,
	 * but should make it safe to work with this class
	 */
	ATVRCharacter(const FObjectInitializer& OI);

	/**
	 * @param OutLifetimeProps Reference to the replicated properties
     */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
	/**
	 * Called after properties are initialized
	 */
	virtual void PostInitProperties() override;
	/**
	 * Called after components are initialized
	 */
	virtual void PostInitializeComponents() override;
	
	/**
	 * Periodic Tick function. Called every frame.
	 * @param DeltaTime Time between last frame in s
	 */
	virtual void Tick(float DeltaTime) override;	

	/**
	 * Called when Actors begins play.
	 */
	virtual void BeginPlay() override;

    virtual void PossessedBy(AController* NewController) override;

    //
    UFUNCTION(Category = "Character", Reliable, Client)
    void ClientPossessed();
    virtual void ClientPossessed_Implementation();

    virtual void SetupHands();

	virtual void RepositionHands(bool bIsRightHand, const FTransform& NewTransform);

	UFUNCTION()
	void OnRightControllerProfileChanged(const FTransform& NewTransformForComps, const FTransform& NewProfileTransform);
	UFUNCTION()
	void OnLeftControllerProfileChanged(const FTransform& NewTransformForComps, const FTransform& NewProfileTransform);
	
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetControllerProfile(bool bIsRightHand, FTransform_NetQuantize NewTransform);
	bool ServerSetControllerProfile_Validate(bool bIsRightHand, FTransform_NetQuantize NewTransform) {return true;}
	void ServerSetControllerProfile_Implementation(bool bIsRightHand, FTransform_NetQuantize NewTransform);

	UPROPERTY(ReplicatedUsing="OnRepRightControllerOffset")
	FTransform_NetQuantize RightControllerOffset;

	UFUNCTION()
	void OnRepRightControllerOffset(FTransform_NetQuantize NewValue);
	
	UPROPERTY(ReplicatedUsing="OnRepLeftControllerOffset")
	FTransform_NetQuantize LeftControllerOffset;

	UFUNCTION()
	void OnRepLeftControllerOffset(FTransform_NetQuantize NewValue);
	
	/**
	 * @returns True when the character is able to move
	 */
	virtual bool CanMove() const;
	
	/**
	* @returns True when the character is able to turn
	*/
	virtual bool CanTurn() const;

	/**
	 * Function used to obtain the forward and right axis for movement, depending on the Gameplay Settings
	 * @param OutAxisRight Reference to the FVector where the forward axis shall be stored (corresponding to the y-axis of the thumbstick)
	 * @param OutAxisForward Reference to the FVector where the right axis shall be stored (corresponding to the x-axis of the thumbstick)
	 */
	virtual void GetMovementAxes(FVector& OutAxisForward, FVector& OutAxisRight) const;

	/**
	 * Returns the appropriate motion controller for the ControllerHand
	 * @param HandType Type of Hand
	 * @return MotionController if there is a result. Nullptr if the input is invalid
	 */
	class UGripMotionControllerComponent* GetControllerHand(EControllerHand HandType) const;

	/**
	 * Returns the appropriate opposite motion controller for the ControllerHand.
	 * Example when the input is Right then the left Hand will be returned
	 * @param HandType Type of the Hand we want the opposite hand for
	 * @return MotionController if there is a result. Nullptr if the input is invalid
	 */
	class UGripMotionControllerComponent* GetOtherControllerHand(EControllerHand HandType) const;
	class UGripMotionControllerComponent* GetOtherControllerHand(UGripMotionControllerComponent* InHand) const;

    virtual bool AttemptToGripObject(UObject* ObjectToGrip, class UGripMotionControllerComponent* Hand, class UGripMotionControllerComponent* OtherHand, const FHitResult& Hit);

	virtual bool AttemptToSecondaryGripObject(const FTransform& GripTransform, UObject* ObjectToGrip, class UGripMotionControllerComponent* Hand, const FHitResult& Hit);
	virtual bool AttemptToPrimaryGripObject(const FTransform& GripTransform, UObject* ObjectToGrip, class UGripMotionControllerComponent* Hand, const FHitResult& Hit);

    virtual bool TryStartClimbing(class UTVRClimbableCapsuleComponent* ClimbableComp, class UGripMotionControllerComponent* GrippingHand);

    
    virtual class UTVRHoverInputVolume* GetOverlappingHoverInputComp(class USphereComponent* GrabSphere) const;
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnGrabLargeLeft();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnGrabLargeRight();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnStopGrabLargeLeft();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnStopGrabLargeRight();    

	/** Axis Input for Gripping. Will replace some functions above for setting up custom dead zones for gripping */
	virtual void OnGrabAxisLeft(float Value);
	/** Axis Input for Gripping. Will replace some functions above for setting up custom dead zones for gripping */
	virtual void OnGrabAxisRight(float Value);
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnUseOrGrabSmallLeft();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnUseOrGrabSmallRight();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnStopUseOrGrabSmallLeft();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
    virtual void OnStopUseOrGrabSmallRight();

	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnTriggerTouchRight();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnTriggerReleaseRight();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnTriggerTouchLeft();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnTriggerReleaseLeft();

    virtual void GrabOrUse(UGripMotionControllerComponent* UsingHand);
    virtual void StopGrabOrUse(UGripMotionControllerComponent* UsingHand);
    virtual void UseHeldObject(UGripMotionControllerComponent* UsingHand, bool bUse);

    UFUNCTION(Category = "Components", BlueprintNativeEvent)
    class USphereComponent* GetRightGrabSphere() const;
    class USphereComponent* GetRightGrabSphere_Implementation() const {return GrabSphereRight;}
    
    UFUNCTION(Category = "Components", BlueprintNativeEvent)
	class USphereComponent* GetLeftGrabSphere() const;
	class USphereComponent* GetLeftGrabSphere_Implementation() const {return GrabSphereLeft;}

	class USphereComponent* GetGrabSphere(UGripMotionControllerComponent* InHand) const;
    
	UFUNCTION(Category = "Inventory", BlueprintImplementableEvent)
	class UTVREquipmentPoint* GetPrimaryWeaponSlot() const;
	UFUNCTION(Category = "Inventory", BlueprintImplementableEvent)
    class UTVREquipmentPoint* GetSidearmSlot() const;


	UFUNCTION(Category = "Inventory", BlueprintCallable)
	virtual float GetSprintStrength() const;


	UPROPERTY(Category = Hand, BlueprintReadWrite)
	UPrimitiveComponent* LeftHandGripComponent;
	UPROPERTY(Category = Hand, BlueprintReadWrite)
	UPrimitiveComponent* RightHandGripComponent;

	ATVRGraspingHand* GetLeftGraspingHand() const;
	ATVRGraspingHand* GetRightGraspingHand() const;
	
	ATVRGraspingHand* GetGraspingHand(EControllerHand HandType) const;
	ATVRGraspingHand* GetGraspingHand(UGripMotionControllerComponent* Controller) const;
	
protected: // Methods

	/** Handles any Movement logic during Tick. Do not call elsewhere */
	virtual void HandleMovement(float DeltaTime);
	
	/** Handles any Turning logic during Tick. Do not call elsewhere. */
	virtual void HandleTurning(float DeltaTime);
	
	/** Snap Turn Right for input binding */
    UFUNCTION()
    void SnapTurnRight();

	/** Snap Turn Left for input binding */
	UFUNCTION()
    void SnapTurnLeft();

	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	void OnAxisTurnX(float Value);

	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	void OnAxisMoveX(float Value);
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	void OnAxisMoveY(float Value);
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	void OnTriggerAxisL(float Value);
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	void OnTriggerAxisR(float Value);
	
	/**
	 * Generic Snap Turn Function. Will use settings from the TVRGameplaySettings class.
	 * This function initiates Snap Turning, by starting to fade out the screen.
	 * @param TurnDir Direction to turn
	 */
	virtual void StartSnapTurn(ETurnDirection TurnDir);
	
	/**
	 * Performs the actually snap turning
	 */
	virtual void PerformSnapTurn();
	
	/**
	 * Finishes snap turning by fading in the screen
	 */
	virtual void FinishSnapTurn();

	/**
	 *
	 */
	virtual void GripLeftPressed();
	virtual void GripLeftReleased();	

	bool IsLocalGrip(EGripMovementReplicationSettings RepType) const;

	UFUNCTION(Category ="Grip", BlueprintCallable)
	virtual bool TryGrip(class UGripMotionControllerComponent* Hand, bool bIsLargeGrip);

	virtual bool TraceGrips(TArray<FHitResult>& OutHits, UGripMotionControllerComponent* Hand, UPrimitiveComponent* OverlapComp);
	virtual bool IsGripValid(UObject* ObjectToGrip, bool bIsLargeGrip, UGripMotionControllerComponent* GripInitiator) const;
	/**
	 *
	 * @returns SG_NONE in case it fails. Otherwise it will return the appropriate grip type
	 */
	virtual ESecondaryGripType CanAttemptSecondaryGrabOnObject(class UObject* ObjectToCheck) const;
	
	/**
	 * @param Hand Hand that initiates the action
	 * @param bIsLargeGrip Is this a large grip
	 * @returns true if the action was handled
	 */
	UFUNCTION(Category ="Grip", BlueprintCallable)
	virtual bool TryDrop(class UGripMotionControllerComponent* Hand, bool bIsLargeGrip);	

	
	virtual bool LocalRemoveSecondaryAttachmentGrip(class UGripMotionControllerComponent* GrippingHand, class UObject* ObjectToRemove);
	UFUNCTION(Reliable, Server, WithValidation)
	virtual void ServerRemoveSecondaryAttachmentGrip(class UGripMotionControllerComponent* GrippingHand, class UObject* ObjectToRemove);
	virtual void ServerRemoveSecondaryAttachmentGrip_Implementation(class UGripMotionControllerComponent* GrippingHand, class UObject* ObjectToRemove);
	virtual bool ServerRemoveSecondaryAttachmentGrip_Validate(class UGripMotionControllerComponent* GrippingHand, class UObject* ObjectToRemove) {return true;}

	/**
	 * Try Grab Function that is client/server agnostic. This will call the appropriate function automatically
	 * @param GripInfo Grip Information Structure
	 * @param HandType Hand Controller Type (Left/Right/other) of the gripping hand
	 */
	UFUNCTION(Category = "Gripping", BlueprintCallable)
	virtual void TryGrabObject(		
		const FGrabObjectInfo& GripInfo,
		EControllerHand HandType
	);

	/**
	 * Try Grab Function that is client/server agnostic. This will call the appropriate function automatically
	 * @param GripInfo Grip Information Structure
	 * @param GrippingHand Hand Motion Controller that is trying to perform the grip
	 * @param OtherHand The Other Hand of the character (assuming that he as two)
	 */
	virtual bool LocalTryGrabObject(
		const FGrabObjectInfo& GripInfo,
		UGripMotionControllerComponent* GrippingHand,
		UGripMotionControllerComponent* OtherHand
	);

	/**
	 * Try Grab Function that is client/server agnostic. This will call the appropriate function automatically
	 * @param GripInfo Grip Information Structure
	 * @param HandType Hand Controller Type (Left/Right/other) of the gripping hand
	 */
	UFUNCTION(Category = "Gripping", BlueprintCallable, Reliable, Server, WithValidation)
	void ServerTryGrabObject(const FGrabObjectInfo& GripInfo, EControllerHand HandType);	
	virtual void ServerTryGrabObject_Implementation(const FGrabObjectInfo& GripInfo, EControllerHand HandType);
	virtual bool ServerTryGrabObject_Validate(const FGrabObjectInfo& GripInfo, EControllerHand HandType) { return true; }

	/**
	 * Try Grab Function that is client/server agnostic. This will call the appropriate function automatically
	 * @param GripInfo Grip Information Structure
	 * @param GrippingHand Hand that is gripping
	 * @param OtherHand The other hand
	 */
	virtual bool TrySecondaryGripObject(const FGrabObjectInfo& GripInfo, UGripMotionControllerComponent* GrippingHand, UGripMotionControllerComponent* OtherHand);

	/**
	 * @param ObjectToGrip Object that should be gripped
	 * @param bHadSlot Is this a slot grip?
	 * @returns Whether the object can be secondary gripped.
	 */
	virtual bool CanSecondaryGripObject(
		UObject* ObjectToGrip,
		bool bHadSlot
	) const;
	
	/**
	 * Calls either the server function or the local one depending on the type of client
	 * @param MotionController Motion Controller that wants to drop
	 * @param Grip Grip Information for this drop
	 */
	UFUNCTION(Category = "Gripping", BlueprintCallable)
	virtual void TryDropObject(class UGripMotionControllerComponent* MotionController, const FBPActorGripInformation& Grip);

	/**
	 * Tries to drop an object
	 * @param MotionController MotionController that tries to perform the action
	 * @param Grip Grip Information of the action (object trying to drop, and how it should be dropped)
	 * @param AngleVel Angular velocity
	 * @param TransVel Translational velocity
	 * @returns True if the action was successful
	 */
	UFUNCTION(Category = "Gripping", BlueprintCallable)
	virtual bool LocalTryDropObject(
		UGripMotionControllerComponent* MotionController,
		const FBPActorGripInformation& Grip,
		const FVector& AngleVel,
		const FVector& TransVel
	);

	/**
	 * Server specific function to try to drop an object. Called from owner if owner is not the server.
	 * @param MotionController MotionController that tries to perform the action
	 * @param AngleVel Angular Velocity
	 * @param TransVel Translational velocity
	 * @param GripHash Compressed hash of the grip information to look up.
	 */
	UFUNCTION(Category = "Gripping", BlueprintCallable, Reliable, Server, WithValidation)
	void ServerTryDropObject(
		UGripMotionControllerComponent* MotionController,
		FVector_NetQuantize100 AngleVel,
		FVector_NetQuantize100 TransVel,
		uint8 GripHash
	);		
	virtual void ServerTryDropObject_Implementation(
		UGripMotionControllerComponent* MotionController,
		FVector_NetQuantize100 AngleVel,
		FVector_NetQuantize100 TransVel,
		uint8 GripHash
	);
	virtual bool ServerTryDropObject_Validate(
		UGripMotionControllerComponent* MotionController,
		FVector_NetQuantize100 AngleVel,
		FVector_NetQuantize100 TransVel,
		uint8 GripHash
	) {return true;}

	/**
	 * Returns the angular and translational throwing velocity based on this characters settings
	 * @param OutAngularVel Angular Velocity Output
	 * @param OutTransVel Translational Velocity Output
	 * @param ThrowingController Motion Controller that is throwing something
	 * @param Grip Grip Information for this throw
	 * @param AngularVelocity Base Angular Velocity
	 * @param TransVelocity Base Translational Velocity
	 */
	virtual void GetThrowingVelocity(
		FVector& OutAngularVel,
		FVector& OutTransVel,
		class UGripMotionControllerComponent* ThrowingController,
		const FBPActorGripInformation& Grip,
		const FVector& AngularVelocity,
		const FVector& TransVelocity
	) const;

	/**
	 * Returns a immutable pointer to the corresponding Filter for the controller hand. For use any const functions.
	 * @param HandType The Type of Controller Hand
	 * @returns immutable pointer to the filter
	 */
	virtual FBPLowPassPeakFilter const* GetHandVelocityFilter(EControllerHand HandType) const;
	
	/**
	* Returns a reference to the corresponding Filter for the controller hand.
	* @param OutVelocity OutputVelocity
	* @param HandType The Type of Controller Hand
	*/
	UFUNCTION(Category="VRCharacter|Hands", BlueprintCallable)
	void GetFilteredHandVelocity(FVector& OutVelocity, EControllerHand HandType) const;

	
	/**
	 * Adds the current velocity to the filters. Call during Tick(), do not call elsewhere.
	 */
	virtual void SampleGripVelocities();
	
	/**
	* Adds the current velocity to the corresponding filter.
	* @param MotionController Motion Controller to read the velocity from.
	* @param Filter Filter to register the new sample in.
	*/
	virtual void SampleGripVelocity(class UGripMotionControllerComponent* MotionController, FBPLowPassPeakFilter& Filter);

	/**
	 * Called by LeftMotionController OnGrippedObject.
	 * @param Grip Grip Information
	 */
	UFUNCTION()
	virtual void OnGrippedObjectLeft(const FBPActorGripInformation& Grip);
	
	/**
	* Called by RightMotionController OnGrippedObject.
	* @param Grip Grip Information
	*/
	UFUNCTION()
	virtual void OnGrippedObjectRight(const FBPActorGripInformation& Grip);
	

    /**
     * Toggles Pause menu based on its current state
     */
	virtual void TogglePauseMenu(UGripMotionControllerComponent* UsingHand);

    /**
     * Event called when the pause menu button is released
     */
	virtual void ReleaseTogglePauseMenu(UGripMotionControllerComponent* UsingHand);

    /**
     * Opens the Pause menu
     */
    UFUNCTION(Category = "Menu", BlueprintCallable)
    virtual void OpenPauseMenu();


	/**
	 * Opens Pause Menu
	 * @returns True if the menu was successfully opened. False is something prevented it.
	 */
	virtual bool TryOpenPauseMenu();

	/**
	 * Close Pause Menu
	 */
    UFUNCTION(Category = "Menu", BlueprintCallable)
	virtual void TryClosePauseMenu();

	/**
	 * @returns Whether pause menu can be opened
	 */
	virtual bool CanOpenPauseMenu();


    UFUNCTION(Category = "Menu", BlueprintImplementableEvent)
    void OnOpenPauseMenu();
    
    UFUNCTION(Category = "Menu", BlueprintImplementableEvent)
    void OnClosePauseMenu();

	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnMagReleasePressed_Left();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnMagReleaseReleased_Left();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnMagReleasePressed_Right();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnMagReleaseReleased_Right();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnBoltReleasePressed_Left();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnBoltReleaseReleased_Left();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnBoltReleasePressed_Right();
	
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnBoltReleaseReleased_Right();

	virtual void OnActionA_Pressed(UGripMotionControllerComponent* UsingHand);
	virtual void OnActionB_Pressed(UGripMotionControllerComponent* UsingHand);
	virtual void OnActionA_Released(UGripMotionControllerComponent* UsingHand);
	virtual void OnActionB_Released(UGripMotionControllerComponent* UsingHand);
    
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnCycleFireMode_Left();
	UFUNCTION(Category = "Character|InputActions", BlueprintCallable)
	virtual void OnCycleFireMode_Right();
	
	virtual void OnCycleFireMode(UGripMotionControllerComponent* UsingHand);


    /**
     * Spawns a new set of Grasping Hands.
     */
    virtual void SpawnGraspingHands();
    
    /**
     * Destroys the grasping hands if they exist
     */
    virtual void DestroyGraspingHands();


    virtual void TickWidgetInteraction(float DeltaTime);


	bool bIsActionAPressed_L;
	bool bIsActionAPressed_R;
	bool bIsActionBPressed_L;
	bool bIsActionBPressed_R;
	
    
protected: // Properties

	FVector2D AxisTurn;
	FVector2D AxisMove;
	float TriggerAxisR;
	float TriggerAxisL;
    
    /** Timer that is active when opening the menu. */
    UPROPERTY(Category = "Menu", BlueprintReadOnly)
    FTimerHandle MenuOpenTimer;
    
    /** Time that the Menu button has to be held until the menu is open */
	UPROPERTY(Category = "Menu", EditDefaultsOnly, BlueprintReadOnly)
    float MenuOpenTime;
    
	/** Timer that is active during snap turn fade in/out */
	FTimerHandle SnapTurnTimerHandle;
	
	/** Pending rotation in degrees that the character wants to turn */
	float PendingTurn;

	/** Pause Menu Cooldown Timer, so that it cannot be spammed to create and destroy actors at will */
	FTimerHandle PauseMenuTimer;
    
	/** Indicates whether the player is currently in menu or not */
	UPROPERTY(Replicated)
	bool bIsInMenu;
    
	/** Pause Menu Actor in case it exists. Will Probably Move this to the player controller? */
    UPROPERTY(Category = "Menu", BlueprintReadWrite)
	class APauseMenuActor* PauseMenuActor;

	/** Velocity Filter for the left hand */
	FBPLowPassPeakFilter PeakVelocityLeft;
	
	/** Velocity Filter for the right hand */
	FBPLowPassPeakFilter PeakVelocityRight;

	/** Sample size of the velocity filters*/
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
    int32 VelocitySampleSize;

	/** Flag that controls whether Grip Velocity should be sampled (actually buffered) or not */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
	bool bSampleGripVelocity;

	/** Flag that controls whether the Controller velocity is used on release for throwing */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
	bool bUseControllerVelocityOnRelease;

	/** Flag that controls whether the throwing velocity is scaled by the mass of the thrown object */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
    bool bScaleThrowingVelocityByMass;

	/** Upper limit for throwing mass. Anything above this mass will be scaled with the minimum throwing mass scale. */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
	float MaxThrowingGripMass;

	/** Minimum scaling factor used for mass scaling of throwing velocity */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
    float MinThrowingMassScale;

	/** Flag that controls whether the maximum throwing velocity shall be limited. */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
    bool bLimitMaxThrowingVelocity;

	/** Upper bound the throwing velocity is limited by */
	UPROPERTY(Category = "Throwing", EditDefaultsOnly)
    float MaxThrowingVelocity;

    /** Whether the Character is already possessed by a controller */
    bool bAlreadyPossessed;

    /** Headset type of the player controlling this character. */
    EBPHMDDeviceType HeadsetType; // TODO: Evaluate if it is really needed 

    /** Class of the Right Grasping Hand Actor. A Grasping hand of this class will be spawned. */
    UPROPERTY(Category = "Interaction", EditDefaultsOnly)
    TSubclassOf<class ATVRGraspingHand> RightGraspingHandClass;


    UPROPERTY(Category = "Menu", BlueprintReadOnly)
    FHitResult WidgetHitResult;

	float PrevTurnX;
	bool bBlockTurn;

	
	UPROPERTY(Category = "Movement", EditDefaultsOnly)
	float SprintMinWeaponDown;
	UPROPERTY(Category = "Movement", EditDefaultsOnly)
	float SprintMaxWeaponDown;
	UPROPERTY(Category = "Movement", EditDefaultsOnly)
	float SprintMinAim;
	UPROPERTY(Category = "Movement", EditDefaultsOnly)
	float SprintMaxAim;

	
	struct FTVRHysteresisValue GrabHysteresisLeft;
	struct FTVRHysteresisValue GrabHysteresisRight;
	
private:

    /** Widget Interaction Component */
    UPROPERTY(Category="Interaction", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    class UWidgetInteractionComponent* WidgetInteraction;

    /** Grab collision sphere of left hand */
    UPROPERTY(Category="Interaction", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    class USphereComponent* GrabSphereLeft;
    
    /** Grab collision sphere of right hand */
    UPROPERTY(Category="Interaction", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    class USphereComponent* GrabSphereRight;

    /** Hand mesh of the right hand */
    UPROPERTY(Category="Interaction", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    class USkeletalMeshComponent* HandMeshRight;
    
    /** Hand mesh of the left hand */
    UPROPERTY(Category="Interaction", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    class USkeletalMeshComponent* HandMeshLeft;

    /** Right Grasping Hand Actor */
    UPROPERTY(Category="Interaction", BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    ATVRGraspingHand* RightGraspingHand;

    /** Left Grasping Hand Actor*/
    UPROPERTY(Category="Interaction", BlueprintReadOnly, meta=(AllowPrivateAccess=true))
    ATVRGraspingHand* LeftGraspingHand;

};
