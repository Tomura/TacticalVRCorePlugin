// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "GripMotionControllerComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/CapsuleComponent.h"
#include "Grippables/HandSocketComponent.h"
#include "Misc/OptionalRepSkeletalMeshActor.h"

#include "TVRGraspingHand.generated.h"


UENUM(BlueprintType)
enum class ETriggerIndices: uint8
{
	Thumb3,
	Thumb2,
	Index3,
	Index2,
	Middle3,
	Middle2,
	Ring3,
	Ring2,
	Pinky3,
	Pinky2
};

UENUM(BlueprintType)
enum class ECurlDirection: uint8
{
	None,
	Forward,
	Reverse
};

UENUM(BlueprintType)
enum class EHandAnimState: uint8
{
	Animated,
    Dynamic,
    Frozen,
	Custom
};

/**
 * 
 */
UCLASS()
class TACTICALVRCORE_API ATVRGraspingHand : public AOptionalRepGrippableSkeletalMeshActor
{
	GENERATED_BODY()

public:
	/** Whether this hand should use finger curling or not */
	UPROPERTY(Category = "Hand", BlueprintReadOnly, meta = (ExposeOnSpawn = true))
	bool bUseCurls;

	/** Whether is hand is a physical hand or not. */
	UPROPERTY(Category = "Hand", BlueprintReadOnly, meta = (ExposeOnSpawn = true))
	bool bIsPhysicalHand;

	/** The Root Bone Name of this hand. */
	UPROPERTY(Category = "Hand", BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FName BoneName;

	/** The hand/controller this actor belongs to */
	UPROPERTY(Category = "Hand", BlueprintReadOnly, meta = (ExposeOnSpawn = true))
	UGripMotionControllerComponent* OwningController;

	/** The other hand of the owning character*/
	UPROPERTY(Category = "Hand", BlueprintReadOnly, meta = (ExposeOnSpawn = true))
	UGripMotionControllerComponent* OtherController;

	/** The current physics root, generally set by the spawning character. */
	UPROPERTY(Category = "Hand", BlueprintReadOnly, meta = (ExposeOnSpawn = true))
	UPrimitiveComponent* PhysicsRoot;

	/** Animation state of the Hand */
	UPROPERTY(Category="Hand", BlueprintReadOnly)
	EHandAnimState HandAnimState;

	/** Proxy primitive for attachment. A bit of a workaround due to a engine bug */
	UPROPERTY(Category="Hand", BlueprintReadOnly)
	UNoRepSphereComponent* AttachmentProxy;

	/** Whether the trigger is touched or not. Will stay true for controllers without touch sensors on the trigger. */
	UPROPERTY(Category="Hand", BlueprintReadOnly)
	bool bIsTriggerTouched;
	
	/** Trigger axis value */
	UPROPERTY(Category="Hand", BlueprintReadOnly)
	float TriggerPress;

protected:	
	/** Hand Type Left/Right of the corresponding motion controller */
	EControllerHand HandType;

	/** The currently gripped Object */
	UPROPERTY(Category= "Hand", BlueprintReadOnly)
	class UObject* GrippedObject;

	/** The Grip ID of the current grip */
	UPROPERTY(Category= "Hand", BlueprintReadOnly)
	uint8 GraspID;

	/** Whether this object is already gripping something */
	UPROPERTY(Category = "Hand", BlueprintReadOnly)
	bool bIsGripping;	
	
	/** The basic relative transform this actor needs to return to, when not attached */
	UPROPERTY(Category= "Hand", BlueprintReadWrite)
	FTransform BaseRelativeTransform;

	/** Transform set at the beginning of lerping */
	UPROPERTY(Category= "Hand", BlueprintReadWrite)
	FTransform BeginLerpTransform;

	/** The Original Grip Transform, when not attached */
	UPROPERTY(Category= "Hand", BlueprintReadWrite)
	FTransform OriginalGripTransform;
	
	/** Current lerp axis value */
	float HandLerpAlpha;		
	UPROPERTY(Category="Hand", EditDefaultsOnly)

	/** Lerp Speed */
	float HandLerpSpeed;
	
	ECurlDirection CurlDirection;
	bool bHandleCurl;
	bool bCurlForward;
	float CurlAlpha;
	UPROPERTY(Category="Hand", EditDefaultsOnly)
	float CurlSpeed;
	UPROPERTY(Category= "Hand", BlueprintReadWrite)
	bool bHadCurled;

	UPROPERTY(Category="Hand", BlueprintReadOnly)
	bool bHasCustomAnimation;
	bool bUseTargetMeshTransform;
	UPROPERTY(Category="Hand", BlueprintReadOnly)
	bool bCustomAnimIsSnapShot;
	UPROPERTY()
	FTransform TargetMeshTransform;
	UPROPERTY(Category="HandSocket", BlueprintReadOnly)
	FPoseSnapshot CustomPose;
	UPROPERTY(Category="Hand", BlueprintReadOnly)
	class UHandSocketComponent* HandSocketComponent;

	/** Collision Capsules for the fingers */
	UPROPERTY(Category = "Hand", BlueprintReadWrite)
	TMap<ETriggerIndices, class UCapsuleComponent*> FingerCollisionZones;
	
	/** Whether the fingers are blocked or not */
	UPROPERTY(Category = "Hand", BlueprintReadWrite)
	TMap<ETriggerIndices, bool> FingersBlocked;

	/** Current Flex values for fingers */
	UPROPERTY(Category = "Hand", BlueprintReadWrite)
	TMap<ETriggerIndices, float> FingerFlex;

	/** Whether fingers are overlapping something or not*/
	UPROPERTY(Category = "Hand", BlueprintReadWrite)
	TMap<ETriggerIndices, bool> FingersOverlapping;
		
private:
	/** Whether the hand is currently being interpolated into position **/
	bool bLerpHand;

	/** A component that marks the location a hand bone needs to be set to */
	UPROPERTY(Category = "Hand", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	USceneComponent* HandBoneLoc;

	UPROPERTY(Category = "Hand", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UOpenXRHandPoseComponent* HandPoseComp;
	
public:
	/**
	 * Constructs and instance of this class. Do not call directly, use UWorld::SpawnActor...
	 */
    ATVRGraspingHand(const FObjectInitializer& OI);

	/**
	 * Called at the beginning of play. Usually right after spawning or when the level starts
	 */
	virtual void BeginPlay() override;

	/**
	 * Called every frame.
	 * @param DeltaSeconds the time since the last frame in seconds
	 */
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Called one frame after being play to finish begin play
	 */
	virtual void DelayedBeginPlay();

	/**
	 * Initializes physics related things, like tick order, etc.
	 * Called during delayed begin play
	 */
	UFUNCTION(Category= "Hand", BlueprintCallable)
	virtual void InitPhysics();

	/**
	 * Called during initialization to setup events relating to finger overlaps
	 */
	virtual void SetupFingerOverlapEvents();

	class ATVRGunBase* GetGrippedGun() const;
	
	UFUNCTION(Category = "Hand", BlueprintImplementableEvent)
	class UPrimitiveComponent* GetRootPhysics() const;
	
	class UPrimitiveComponent* GetPhysicsRoot() const {return PhysicsRoot; }
	
	UFUNCTION(Category = "Hand", BlueprintImplementableEvent)
    class UVREPhysicsConstraintComponent* GetSimulatingHandConstraint() const;

	UFUNCTION(Category = "Hand", BlueprintImplementableEvent)
    class UVREPhysicalAnimationComponent* GetPhysicalAnimation() const;

	UFUNCTION(Category= "Hand", BlueprintCallable)
	class ATVRCharacter* GetOwnerCharacter() const;
	
	virtual void EvaluateGrasping();
	virtual void HandleCurls(float GripCurl, ECurlDirection Direction);
	virtual void FingerMovement(ETriggerIndices FingerKey, float AxisInput);
	
	virtual void SetDynamicFingerCurls();

	virtual void GetOrSpawnAttachmentProxy();
	virtual void ResetAttachmentProxy();	

	bool IsGrippedObjectWeapon() const;

	UFUNCTION(Category="Hand", BlueprintImplementableEvent)
	void PostHandleGripped();
	
	virtual void InitializeAndAttach(const FBPActorGripInformation& GripInfo, bool bIsSecondaryGrip, bool bSkipEvaluation = false);
protected:
	
	UFUNCTION(Category="Hand", BlueprintCallable)
	void SetFingerOverlaps(bool bEnableOverlaps);

	virtual void SetWeaponCollisionResponse(ECollisionResponse NewResponse);

	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void OnGrippedObject(const struct FBPActorGripInformation& GripInfo);
	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void OnDroppedObject(const struct FBPActorGripInformation& GripInfo, bool bWasSocketed);
	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void OnSecondaryRemovedOnOther(const struct FBPActorGripInformation& GripInfo);
	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void OnSecondaryAddedOnOther(const struct FBPActorGripInformation& GripInfo);

	UFUNCTION()
	virtual void OnOwnerTeleported();
	virtual void DelayedOwnerTeleported();
	virtual void FinishOwnerTeleported();

	virtual void SetFingerOverlapping(bool bOverlapping, ETriggerIndices FingerKey, AActor* Actor, UPrimitiveComponent* Comp);
	
	/**
	 * Active Physics wrapped in a function to call after one frame
	 */
	virtual void DelayedActivePhysics();

	/**
	 * Start Lerping Hand
	 */
	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void StartLerpHand();

	/**
	 * Stops Lerping Hand
	 */
	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void StopLerpHand();

	/**
	 * Handles Lerping Hand during Tick
	 * @param DeltaTime time between the last frame in seconds
	 */
	virtual void UpdateLerpHand(float DeltaTime);
	virtual void FinishedLerpHand();

	virtual void StartCurl();
	virtual void StopCurl();
	virtual void ReverseCurl();
	virtual void ResetCurl();
	virtual void UpdateCurl(float DeltaTime);

	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void SetupPhysicsIfNeededNative(bool bSimulate, bool bSetRelativeTrans);

	// virtual void InitializeAndAttach(const FBPActorGripInformation& GripInfo, bool bIsSecondaryGrip, bool bSkipEvaluation = false);

	virtual void RetrievePoses(const FBPActorGripInformation& GripInfo, bool bIsSecondary);
	
	UFUNCTION(Category="Hand", BlueprintCallable)
	virtual void SetFingerCollisions();

	UFUNCTION(Category="Hand", BlueprintCallable)
    virtual void ClearFingers();	
	
	UFUNCTION(Category="Hand", BlueprintImplementableEvent)	
	void BPSetFingerCollisions();

	void SetPhysicalRelativeTransform();
	
	UFUNCTION()
	void OnThumb03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnThumb03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnThumb02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnThumb02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnIndex03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnIndex03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnIndex02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnIndex02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnMiddle03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnMiddle03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnMiddle02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnMiddle02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRing03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnRing03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnRing02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnRing02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnPinky03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnPinky03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnPinky02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit);
	UFUNCTION()
	void OnPinky02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	
};
