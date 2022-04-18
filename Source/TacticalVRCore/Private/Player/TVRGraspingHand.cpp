// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/TVRGraspingHand.h"
#include "Player/TVRCharacter.h"
#include "VRBPDatatypes.h"
#include "Weapon/TVRGunBase.h"
#include "TacticalTraceChannels.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/VREPhysicsConstraintComponent.h"
#include "Misc/VREPhysicalAnimationComponent.h"
// #include "SteamVRInputDeviceFunctionLibrary.h"
#include "OpenXRHandPoseComponent.h"
#include "Grippables/HandSocketComponent.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "Weapon/Attachments/WPNA_Foregrip.h"

ATVRGraspingHand::ATVRGraspingHand(const FObjectInitializer& OI) : Super(OI)
{
    bUseCurls = false;
    OwningController = nullptr;
    OtherController = nullptr;
    PhysicsRoot = nullptr;

	bIsGripping = false;
	
    HandBoneLoc = CreateDefaultSubobject<USceneComponent>(FName("HandBoneLoc"));
    HandBoneLoc->SetupAttachment(GetSkeletalMeshComponent());

	HandPoseComp = CreateDefaultSubobject<UOpenXRHandPoseComponent>(FName("HandPoseComponent"));

	BeginLerpTransform = FTransform();
	OriginalGripTransform = FTransform();
	BaseRelativeTransform = FTransform();
	HandLerpAlpha = 0.f;
	HandLerpSpeed = 5.f;
	bLerpHand = false;
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	CurlAlpha = 0.f;
	CurlSpeed = 5.f;
	bHandleCurl = false;
	bCurlForward = false;
	CurlDirection = ECurlDirection::None;

	bHadCurled = false;

	HandType = EControllerHand::Right;
	HandAnimState = EHandAnimState::Animated;
	AttachmentProxy = nullptr;

	bHasCustomAnimation = false;
	bUseTargetMeshTransform = false;
	bCustomAnimIsSnapShot = false;
	HandSocketComponent = nullptr;

	bIsTriggerTouched = true;
	TriggerPress = 0.f;
}

void ATVRGraspingHand::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true); // getHMDType(DT_Steam_VR) && bUseCurls
	if(!OwningController)
	{
		// maybe even destroy
		return;
	}	
	OwningController->GetHandType(HandType);
	GetOrSpawnAttachmentProxy();
	const bool IsLeftHand = GetSkeletalMeshComponent()->DoesSocketExist(FName("hand_l")) && HandType == EControllerHand::Left;
	if(GetPhysicsRoot())
	{
		const FName PalmName = IsLeftHand ? FName("palm_l") : FName("palm_r");
		const FVector NewLoc = GetSkeletalMeshComponent()->GetSocketLocation(PalmName);
		GetPhysicsRoot()->SetWorldLocation(NewLoc);
	}
	BoneName = IsLeftHand ? FName("hand_l") : FName("hand_r");
	GetWorldTimerManager().SetTimerForNextTick(this, &ATVRGraspingHand::DelayedBeginPlay);
	
}

void ATVRGraspingHand::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLerpHand(DeltaSeconds);
	UpdateCurl(DeltaSeconds);

	if(bUseCurls && HandAnimState == EHandAnimState::Animated)
	{
		SetDynamicFingerCurls();
	}
}

void ATVRGraspingHand::DelayedBeginPlay()
{
	BaseRelativeTransform = GetSkeletalMeshComponent()->GetRelativeTransform();
	SetFingerCollisions();
	SetupFingerOverlapEvents();
	GetSkeletalMeshComponent()->SetTickGroup(TG_PrePhysics);

	if(OwningController && OtherController)
	{
		OwningController->OnGrippedObject.AddDynamic(this, &ATVRGraspingHand::OnGrippedObject);
		OwningController->OnDroppedObject.AddDynamic(this, &ATVRGraspingHand::OnDroppedObject);
		OtherController->OnSecondaryGripAdded.AddDynamic(this, &ATVRGraspingHand::OnSecondaryAddedOnOther);
		OtherController->OnSecondaryGripRemoved.AddDynamic(this, &ATVRGraspingHand::ATVRGraspingHand::OnSecondaryRemovedOnOther);
	}
	else
	{
		// if there is no owner, there is not point for this one to exist
		Destroy();
		return;
	}

	SetupPhysicsIfNeededNative(true, false);
	InitPhysics();
}

void ATVRGraspingHand::SetupFingerOverlapEvents()
{
	{
		UCapsuleComponent* Thumb03 = *FingerCollisionZones.Find(ETriggerIndices::Thumb3);
		Thumb03->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnThumb03BeginOverlap);
		Thumb03->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnThumb03EndOverlap);
	}
	
	{
		UCapsuleComponent* Thumb02 = *FingerCollisionZones.Find(ETriggerIndices::Thumb2);
		Thumb02->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnThumb02BeginOverlap);
		Thumb02->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnThumb02EndOverlap);
	}
	
	{
		UCapsuleComponent* Index03 = *FingerCollisionZones.Find(ETriggerIndices::Index3);
		Index03->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnIndex03BeginOverlap);
		Index03->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnIndex03EndOverlap);
	}
	
	{
		UCapsuleComponent* Index02 = *FingerCollisionZones.Find(ETriggerIndices::Index2);
		Index02->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnIndex02BeginOverlap);
		Index02->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnIndex02EndOverlap);
	}

	{
		UCapsuleComponent* Middle03 = *FingerCollisionZones.Find(ETriggerIndices::Middle3);
		Middle03->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnMiddle03BeginOverlap);
		Middle03->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnMiddle03EndOverlap);
	}
	
	{
		UCapsuleComponent* Middle02 = *FingerCollisionZones.Find(ETriggerIndices::Middle2);
		Middle02->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnMiddle02BeginOverlap);
		Middle02->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnMiddle02EndOverlap);
	}

	{
		UCapsuleComponent* Ring03 = *FingerCollisionZones.Find(ETriggerIndices::Ring3);
		Ring03->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnRing03BeginOverlap);
		Ring03->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnRing03EndOverlap);
	}
	
	{
		UCapsuleComponent* Ring02 = *FingerCollisionZones.Find(ETriggerIndices::Ring2);
		Ring02->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnRing02BeginOverlap);
		Ring02->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnRing02EndOverlap);
	}
	
	{
		UCapsuleComponent* Pinky03 = *FingerCollisionZones.Find(ETriggerIndices::Pinky3);
		Pinky03->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnPinky03BeginOverlap);
		Pinky03->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnPinky03EndOverlap);
	}
	
	{
		UCapsuleComponent* Pinky02 = *FingerCollisionZones.Find(ETriggerIndices::Pinky2);
		Pinky02->OnComponentBeginOverlap.AddDynamic(this, &ATVRGraspingHand::OnPinky02BeginOverlap);
		Pinky02->OnComponentEndOverlap.AddDynamic(this, &ATVRGraspingHand::OnPinky02EndOverlap);
	}
}

ATVRGunBase* ATVRGraspingHand::GetGrippedGun() const
{
	if(GrippedObject)
	{
		return Cast<ATVRGunBase>(GrippedObject);
	}
	return nullptr;
}


void ATVRGraspingHand::OnThumb03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Thumb3, OtherActor, OtherComp);
	SetFingerOverlapping(true, ETriggerIndices::Thumb2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnThumb03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Thumb3, OtherActor, OtherComp);
	SetFingerOverlapping(false, ETriggerIndices::Thumb2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnThumb02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Thumb2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnThumb02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Thumb2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnIndex03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Index3, OtherActor, OtherComp);
	SetFingerOverlapping(true, ETriggerIndices::Index2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnIndex03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Index3, OtherActor, OtherComp);
	SetFingerOverlapping(false, ETriggerIndices::Index2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnIndex02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Index2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnIndex02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Index2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnMiddle03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Middle3, OtherActor, OtherComp);
	SetFingerOverlapping(true, ETriggerIndices::Middle2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnMiddle03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Middle3, OtherActor, OtherComp);
	SetFingerOverlapping(false, ETriggerIndices::Middle2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnMiddle02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Middle2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnMiddle02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Middle2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnRing03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Ring3, OtherActor, OtherComp);
	SetFingerOverlapping(true, ETriggerIndices::Ring2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnRing03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Ring3, OtherActor, OtherComp);
	SetFingerOverlapping(false, ETriggerIndices::Ring2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnRing02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Ring2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnRing02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Ring2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnPinky03BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Pinky3, OtherActor, OtherComp);
	SetFingerOverlapping(true, ETriggerIndices::Pinky2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnPinky03EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Pinky3, OtherActor, OtherComp);
	SetFingerOverlapping(false, ETriggerIndices::Pinky2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnPinky02BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bBlockingHit, const FHitResult& Hit)
{
	SetFingerOverlapping(true, ETriggerIndices::Pinky2, OtherActor, OtherComp);
}

void ATVRGraspingHand::OnPinky02EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFingerOverlapping(false, ETriggerIndices::Pinky2, OtherActor, OtherComp);
}

void ATVRGraspingHand::InitPhysics()
{
	ATVRCharacter* OwnerChar = GetOwnerCharacter();
	if(GetPhysicsRoot() && OwnerChar != nullptr && !OwnerChar->IsPendingKill())
	{
		GetPhysicalAnimation()->AddTickPrerequisiteComponent(GetSkeletalMeshComponent());
		GetSkeletalMeshComponent()->AddTickPrerequisiteComponent(OwnerChar->VRMovementReference);
		// OwnerChar->Torso->AddTickPrerequisiteActor(this);
		// OwnerChar->Torso->AddTickPrerequisiteComponent(HandBoneLoc);	
		// OwnerChar->Torso->SetTickGroup(TG_PostPhysics);

		OwnerChar->OnCharacterTeleported_Bind.AddDynamic(this, &ATVRGraspingHand::OnOwnerTeleported);

		if(HandType == EControllerHand::Left)
		{
			OwnerChar->LeftHandGripComponent = GetRootPhysics();
		}
		else
		{
			OwnerChar->RightHandGripComponent = GetRootPhysics();
		}
		OwningController->SetCustomPivotComponent(GetRootPhysics());
		GetRootPhysics()->SetWorldTransform(GetPhysicsRoot()->GetComponentTransform());
		OriginalGripTransform = GetRootPhysics()->GetRelativeTransform();
	}
}

ATVRCharacter* ATVRGraspingHand::GetOwnerCharacter() const
{
	return Cast<ATVRCharacter>(OwningController->GetOwner());
}

void ATVRGraspingHand::EvaluateGrasping()
{
	if(bIsGripping)
	{
		FBPActorGripInformation GripInfo;
		EBPVRResultSwitch Result;
		OwningController->GetGripByID(GripInfo, GraspID, Result);
		if(Result == EBPVRResultSwitch::OnSucceeded)
		{
			for(auto It = FingerCollisionZones.CreateConstIterator(); It; ++It)
			{
				const ETriggerIndices Key = It.Key();
				const bool bFingerBlocked = *FingersBlocked.Find(Key);
				if(!bFingerBlocked)
				{
					bool bInitialOverlap = false;
					if(GripInfo.GripTargetType == EGripTargetType::ActorGrip)
					{
						AActor* GrippedActor = Cast<AActor>(GripInfo.GrippedObject);
						bInitialOverlap = It.Value()->IsOverlappingActor(GrippedActor);
						if(!bInitialOverlap)
						{
							if(const auto TestGun = Cast<ATVRGunBase>(GrippedActor))
							{							
								if(const auto TestForeGrip = TestGun->GetAttachment<AWPNA_ForeGrip>())
								{									
									bInitialOverlap = It.Value()->IsOverlappingActor(TestForeGrip);
								}
							}
						}
					}
					else // EGripTargetType::ComponentGrip
					{
						const auto GrippedComp = Cast<UPrimitiveComponent>(GripInfo.GrippedObject);
						bInitialOverlap = It.Value()->IsOverlappingComponent(GrippedComp);
					}
					if(bInitialOverlap)
					{
						FingersBlocked.Add(Key, true);
						FingersOverlapping.Add(Key, true);
					}
				}
			}
		}
	}
}

void ATVRGraspingHand::HandleCurls(float GripCurl, ECurlDirection Direction)
{
	const float CurrentCurl = GripCurl;
	for(auto It = FingersBlocked.CreateConstIterator(); It; ++It)
	{
		const ETriggerIndices Key = It.Key();
		const bool bFingerBlocked = It.Value();
		const bool bOverlapping = *FingersOverlapping.Find(Key);
		const float Flex = *FingerFlex.Find(Key);
		
		const bool bDoNotCurlFinger = Direction == ECurlDirection::Reverse && CurrentCurl > Flex;
		const bool bDoNotCurlFinger_StartAtCurrentCurl = bDoNotCurlFinger
			|| (bUseCurls && CurrentCurl < Flex && Direction != ECurlDirection::Reverse);
		if(!bDoNotCurlFinger)
		{
			if(bFingerBlocked)
			{
				if(CurrentCurl < Flex || !bOverlapping)
				{ // we need to free the finger again
					FingersBlocked.Add(Key, false);
				}
				else
				{
					// blocked and overlapping and the desired flex is lower thus the hand is opening
					continue;
				}
			}
			FingerMovement(Key, CurrentCurl);
		}
	}
}

void ATVRGraspingHand::FingerMovement(ETriggerIndices FingerKey, float AxisInput)
{
	const float Flex = *FingerFlex.Find(FingerKey);
	if(FMath::Abs(AxisInput-Flex) > 0.05f)
	{
		FingerFlex.Add(FingerKey, AxisInput);
	}
}

void ATVRGraspingHand::SetDynamicFingerCurls()
{
	// FSteamVRFingerCurls Curls;
	// FSteamVRFingerSplays Splays;
	// USteamVRInputDeviceFunctionLibrary::GetFingerCurlsAndSplays(
	// 	HandType == EControllerHand::Left ? EHand::VR_LeftHand : EHand::VR_RightHand,
	// 	Curls, Splays, ESkeletalSummaryDataType::VR_SummaryType_FromAnimation
	// );
	// FingerFlex.Add(ETriggerIndices::Thumb3, Curls.Thumb);
	// FingerFlex.Add(ETriggerIndices::Thumb2, Curls.Thumb);	
	// FingerFlex.Add(ETriggerIndices::Index3, Curls.Index);
	// FingerFlex.Add(ETriggerIndices::Index2, Curls.Index);
	// FingerFlex.Add(ETriggerIndices::Middle3, Curls.Middle);
	// FingerFlex.Add(ETriggerIndices::Middle2, Curls.Middle);
	// FingerFlex.Add(ETriggerIndices::Ring3, Curls.Ring);
	// FingerFlex.Add(ETriggerIndices::Ring2, Curls.Ring);
	// FingerFlex.Add(ETriggerIndices::Pinky3, Curls.Pinky);
	// FingerFlex.Add(ETriggerIndices::Pinky2, Curls.Pinky);
}

void ATVRGraspingHand::GetOrSpawnAttachmentProxy()
{
	if(GetLocalRole() == ROLE_Authority)
	{
		TArray<UNoRepSphereComponent*> Spheres;
		OwningController->GetOwner()->GetComponents<UNoRepSphereComponent>(Spheres);
		const FName SearchTag = HandType == EControllerHand::Left ? FName(TEXT("ATTACHPROXYLEFT")) : FName(TEXT("ATTACHPROXYRIGHT"));
		for(UNoRepSphereComponent* TestComp: Spheres)
		{
			if(TestComp->ComponentHasTag(SearchTag))
			{
				AttachmentProxy = TestComp;
				break;
			}
		}
		if(AttachmentProxy == nullptr)
		{
			// Spawn proxy
			UActorComponent* NewComp = OwningController->GetOwner()->AddComponentByClass(UNoRepSphereComponent::StaticClass(), true, FTransform::Identity, false);
			if(NewComp)
			{
				AttachmentProxy = Cast<UNoRepSphereComponent>(NewComp);
				AttachmentProxy->SetSphereRadius(4.f);
				AttachmentProxy->SetCollisionObjectType(ECC_WorldDynamic);
				AttachmentProxy->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
				AttachmentProxy->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
				AttachmentProxy->SetAllMassScale(0.f);
				AttachmentProxy->AttachToComponent(GetRootPhysics(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				AttachmentProxy->ComponentTags.AddUnique(SearchTag);
			}
		}
	}
}

void ATVRGraspingHand::ResetAttachmentProxy()
{
	if(AttachmentProxy)
	{
		AttachmentProxy->AttachToComponent(OwningController, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}


bool ATVRGraspingHand::IsGrippedObjectWeapon() const
{
	if(GrippedObject)
	{
		if(GrippedObject->GetClass()->IsChildOf(ATVRGunBase::StaticClass()))
		{
			return true;
		}
		if(UActorComponent* Comp = Cast<UActorComponent>(GrippedObject))
		{
			if(Comp->GetOwner() && Comp->GetOwner()->GetClass()->IsChildOf(ATVRGunBase::StaticClass()))
			{
				return true;
			}
		}
	}
	return false;
}

void ATVRGraspingHand::SetFingerOverlaps(bool bEnableOverlaps)
{
	for(auto It = FingerCollisionZones.CreateConstIterator(); It; ++It)
	{
		if(UCapsuleComponent* Comp = It.Value())
		{
			Comp->SetCollisionEnabled(bEnableOverlaps ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		}
	}
}

void ATVRGraspingHand::SetWeaponCollisionResponse(ECollisionResponse NewResponse)
{
	if(!GetSkeletalMeshComponent()->IsPendingKill())
	{
		GetSkeletalMeshComponent()->SetCollisionResponseToChannel(ECC_WeaponObjectChannel, NewResponse);
	}
}

void ATVRGraspingHand::OnGrippedObject(const FBPActorGripInformation& GripInfo)
{
	if(bIsGripping)
	{
		return;
	}

	bIsGripping = true;
	RetrievePoses(GripInfo,false);
	InitializeAndAttach(GripInfo, false, false);

	StopLerpHand();
	if(bHasCustomAnimation)		
	{
		HandAnimState = bHasCustomAnimation ? EHandAnimState::Custom : EHandAnimState::Dynamic;		
	}
	StartCurl();
	// BP_StartHandCurl();
	PostHandleGripped();
}

void ATVRGraspingHand::OnDroppedObject(const FBPActorGripInformation& GripInfo, bool bWasSocketed)
{
	AttachToComponent(OwningController,  FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	HandSocketComponent = nullptr;
	if(GripInfo.GripID == GraspID)
	{
		if(IsGrippedObjectWeapon())
		{
			SetWeaponCollisionResponse(ECollisionResponse::ECR_Block);
		}
		bIsGripping = false;
		SetFingerOverlaps(false);
		BeginLerpTransform = GetSkeletalMeshComponent()->GetRelativeTransform();
		if(GetPhysicsRoot())
		{
			GetRootPhysics()->AttachToComponent(GetSkeletalMeshComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
			GetRootPhysics()->SetRelativeTransform(OriginalGripTransform);
		}
		StartLerpHand();

		if(bIsPhysicalHand)
		{
			GetSkeletalMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HandAnimState = EHandAnimState::Animated;
			bHadCurled = false;
			ClearFingers();
			StopCurl();
			ResetCurl();
			// BP_StopHandCurl();
		}
		else
		{
			HandAnimState = EHandAnimState::Dynamic;
			ReverseCurl();
			// BP_ReverseHandCurl();
		}
		ResetAttachmentProxy();
	}
}

void ATVRGraspingHand::OnSecondaryRemovedOnOther(const FBPActorGripInformation& GripInfo)
{	
	OnDroppedObject(GripInfo, false);
}

void ATVRGraspingHand::OnSecondaryAddedOnOther(const FBPActorGripInformation& GripInfo)
{
	if(!bIsGripping)
	{
		bIsGripping = true;
		SetFingerOverlaps(true);
		GrippedObject = GripInfo.GrippedObject;
		GraspID = GripInfo.GripID;
		if(IsGrippedObjectWeapon())
		{
			SetWeaponCollisionResponse(ECollisionResponse::ECR_Overlap);
		}
		RetrievePoses(GripInfo, true);
		InitializeAndAttach(GripInfo, true, false);
		
		StopLerpHand();
		if(bHasCustomAnimation)		
		{
			HandAnimState = bHasCustomAnimation ? EHandAnimState::Custom : EHandAnimState::Dynamic;		
		}
		StartCurl();
	}
}

void ATVRGraspingHand::OnOwnerTeleported()
{
	if(bIsPhysicalHand && !OwningController->HasGrippedObjects())
	{
		// we need to wait for post-physics
		GetWorldTimerManager().SetTimerForNextTick(this, &ATVRGraspingHand::DelayedOwnerTeleported);
	}
}

void ATVRGraspingHand::DelayedOwnerTeleported()
{
	const FTransform NewTransform = BaseRelativeTransform * OwningController->GetComponentTransform();
	GetSkeletalMeshComponent()->SetSimulatePhysics(false);
	SetActorTransform(NewTransform, false, nullptr, ETeleportType::TeleportPhysics);
	GetSkeletalMeshComponent()->AttachToComponent(OwningController,  FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));

	// we need to let velocities and constrants clear before we start simulation.
	GetWorldTimerManager().SetTimerForNextTick(this, &ATVRGraspingHand::FinishOwnerTeleported);
}

void ATVRGraspingHand::FinishOwnerTeleported()
{
	GetSkeletalMeshComponent()->SetSimulatePhysics(true);
}

void ATVRGraspingHand::SetFingerOverlapping(bool bOverlapping, ETriggerIndices FingerKey, AActor* Actor, UPrimitiveComponent* Comp)
{
	bool bIsGrippedObject = GrippedObject == Actor || GrippedObject == Comp;
	if(!bIsGrippedObject && Actor)
	{
		if(ATVRWeaponAttachment* TestAttachment = Cast<ATVRWeaponAttachment>(Actor))
		{
			bIsGrippedObject = GrippedObject == TestAttachment->GetGunOwner();
		}
	}
	
	if(bIsGrippedObject)
	{
		FingersOverlapping.Add(FingerKey, bOverlapping);
		if(bOverlapping)
		{
			FingersBlocked.Add(FingerKey, true);
		}
	}
}


void ATVRGraspingHand::DelayedActivePhysics()
{
	SetupPhysicsIfNeededNative(true, true);
}

void ATVRGraspingHand::StartLerpHand()
{
	bLerpHand = true;
	HandLerpAlpha = 0.f;
}

void ATVRGraspingHand::StopLerpHand()
{
	bLerpHand = false;
}

void ATVRGraspingHand::UpdateLerpHand(float DeltaTime)
{
	if(bLerpHand)
	{
		const float LerpTarget = 1.f;
		HandLerpAlpha = FMath::FInterpConstantTo(HandLerpAlpha, LerpTarget, DeltaTime, HandLerpSpeed);
		const FTransform NewTransform = UKismetMathLibrary::TLerp(BeginLerpTransform, BaseRelativeTransform, HandLerpAlpha);
		GetSkeletalMeshComponent()->SetRelativeTransform(NewTransform, false, nullptr, ETeleportType::TeleportPhysics);
		if(HandLerpAlpha == LerpTarget)
		{
			bLerpHand = false;
			FinishedLerpHand();
		}
	}
	
}

void ATVRGraspingHand::FinishedLerpHand()
{
	if(bIsPhysicalHand)
	{
		SetPhysicalRelativeTransform();
		GetWorldTimerManager().SetTimerForNextTick(this, &ATVRGraspingHand::DelayedActivePhysics);
	}
}

void ATVRGraspingHand::StartCurl()
{
	HandAnimState = bHasCustomAnimation ? EHandAnimState::Custom : EHandAnimState::Dynamic;
	CurlDirection = ECurlDirection::Forward;
}

void ATVRGraspingHand::StopCurl()
{
	HandAnimState = EHandAnimState::Animated;
	CurlAlpha = 0.f;
	CurlDirection = ECurlDirection::None;
}

void ATVRGraspingHand::ReverseCurl()
{
	HandAnimState = bHasCustomAnimation ? EHandAnimState::Custom : EHandAnimState::Dynamic;
	CurlDirection = ECurlDirection::Reverse;
}

void ATVRGraspingHand::ResetCurl()
{
	CurlAlpha = 0.f;
}

void ATVRGraspingHand::UpdateCurl(float DeltaTime)
{
	if(CurlDirection != ECurlDirection::None)
	{
		const float CurlTarget = (CurlDirection == ECurlDirection::Forward) ? 1.f : 0.f;
		CurlAlpha = FMath::FInterpConstantTo(CurlAlpha, CurlTarget, DeltaTime, CurlSpeed);
		EvaluateGrasping();
		HandleCurls(CurlAlpha, CurlDirection);
		
		if(CurlAlpha == CurlTarget)
		{
			bHadCurled = false;
			CurlDirection = ECurlDirection::None;
			HandAnimState = (CurlDirection == ECurlDirection::Forward) ?
				EHandAnimState::Animated :
				(bHasCustomAnimation ? EHandAnimState::Custom : EHandAnimState::Frozen);
		}
	}
}


void ATVRGraspingHand::SetupPhysicsIfNeededNative(bool bSimulate, bool bSetRelativeTrans)
{
	if(bSimulate)
	{
		if(GetPhysicsRoot())
		{
			USkeletalMeshComponent* SkelMesh = GetSkeletalMeshComponent();
			SkelMesh->SetAllMassScale(1.f);
			SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			SkelMesh->SetSimulatePhysics(true);
			const FVector CenterOfMass = SkelMesh->GetCenterOfMass();
			GetSimulatingHandConstraint()->SetWorldLocation(CenterOfMass, false, nullptr, ETeleportType::TeleportPhysics);
			GetSimulatingHandConstraint()->SetConstrainedComponents(GetPhysicsRoot(), NAME_None, SkelMesh, BoneName);

			// const bool bSetReferenceFrame = SkelMesh->GetComponentScale().GetMin() < 0.f && !bSetRelativeTrans;
			// if(bSetReferenceFrame)
			// {
			// 	FTransform FrameTransform;
			// 	GetSimulatingHandConstraint()->GetConstraintReferenceFrame(EConstraintFrame::Frame1, FrameTransform);
			// 	GetSimulatingHandConstraint()->SetConstraintReferenceFrame(EConstraintFrame::Frame1,
			// 		FTransform(
			// 			FRotator(0.f, 0.f, -180.f).Quaternion() * FrameTransform.GetRotation(),
			// 			FrameTransform.GetLocation(),
			// 			FrameTransform.GetScale3D()
			// 		)
			// 	);
			// }
			GetSimulatingHandConstraint()->SetConstraintToForceBased(true);
			OwningController->bDisableLowLatencyUpdate = true;
			
			if(GetPhysicalAnimation())
			{
				if(bIsPhysicalHand)
				{
					GetPhysicalAnimation()->RefreshWeldedBoneDriver();
				}
				else
				{
					GetPhysicalAnimation()->SetSkeletalMeshComponent(SkelMesh);
					TArray<FName> BoneNames;
					BoneNames.Add(BoneName);
					GetPhysicalAnimation()->SetupWeldedBoneDriver(BoneNames);
					bIsPhysicalHand = true;
				}
			}
		}
	}
	else
	{
		GetSimulatingHandConstraint()->BreakConstraint();		
		USkeletalMeshComponent* SkelMesh = GetSkeletalMeshComponent();
		SkelMesh->SetSimulatePhysics(false);
		if(bIsPhysicalHand && GetPhysicalAnimation())
		{
			GetPhysicalAnimation()->RefreshWeldedBoneDriver();
			// SkelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		} else
		{
			SkelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		SkelMesh->SetAllMassScale(0.f);
	}
}

void ATVRGraspingHand::InitializeAndAttach(const FBPActorGripInformation& GripInfo, bool bIsSecondaryGrip,
	bool bSkipEvaluation)
{
	SetFingerOverlaps(true);
	GrippedObject = GripInfo.GrippedObject;
	GraspID = GripInfo.GripID;
	if(GetPhysicsRoot())
	{
		const FTransform RootTransform = GetPhysicsRoot()->GetComponentTransform();
		FAttachmentTransformRules AttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		AttachRule.bWeldSimulatedBodies = true;
		GetRootPhysics()->AttachToComponent(GetPhysicsRoot(), AttachRule);
	}
	if(bSkipEvaluation)
	{
		EvaluateGrasping();
	}
	FAttachmentTransformRules AttachRule = FAttachmentTransformRules::KeepWorldTransform;
	AttachRule.bWeldSimulatedBodies = true;
	if(GripInfo.GripTargetType == EGripTargetType::ActorGrip)
	{		
		AActor* GrippedActor = Cast<AActor>(GripInfo.GrippedObject);
		AttachmentProxy->AttachToComponent(GrippedActor->GetRootComponent(), AttachRule, GripInfo.GrippedBoneName);
	}
	else // GripInfo.GripTargetType == EGripTargetType::ComponentGrip
	{
		UPrimitiveComponent* GrippedComp = Cast<UPrimitiveComponent>(GripInfo.GrippedObject);
		AttachmentProxy->AttachToComponent(GrippedComp, AttachRule, GripInfo.GrippedBoneName);
	}
	FTransform AttachTransform;
	FTransform ScalerTransform = BaseRelativeTransform;
	ScalerTransform.SetScale3D(FVector::OneVector);

	if(bUseTargetMeshTransform) // we have a hand socket
	{
		AttachmentProxy->SetWorldLocationAndRotation(
			TargetMeshTransform.GetLocation(),
			TargetMeshTransform.Rotator(),
			false, nullptr,
			ETeleportType::TeleportPhysics
		);
	}
	else
	{
		if (!bIsSecondaryGrip) {
			const FTransform A = ScalerTransform * OwningController->GetComponentTransform();
			const FTransform GripRelTransform = bIsSecondaryGrip ?
				GripInfo.SecondaryGripInfo.SecondaryRelativeTransform.Inverse() :
				GripInfo.RelativeTransform;
			const FTransform Rel = GripRelTransform * OwningController->GetPivotTransform();	
			const FTransform NewTransform = A.GetRelativeTransform(Rel);
			AttachmentProxy->SetRelativeTransform(NewTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
		else // here we want to keep the world position
		{
			AttachmentProxy->SetWorldTransform(GetSkeletalMeshComponent()->GetComponentTransform());
			AttachmentProxy->SetWorldScale3D(FVector::OneVector);
		}
	}

	FAttachmentTransformRules AttachRuleHandToProxy = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	AttachRuleHandToProxy.bWeldSimulatedBodies = true;
	AttachToComponent(AttachmentProxy, AttachRuleHandToProxy);
	SetupPhysicsIfNeededNative(false, false);
}

void ATVRGraspingHand::RetrievePoses(const FBPActorGripInformation& GripInfo, bool bIsSecondary)
{
	const bool bIsSlotGrip = bIsSecondary ? GripInfo.SecondaryGripInfo.bIsSlotGrip : GripInfo.bIsSlotGrip;
	if(bIsSlotGrip)
	{
		const FName GrippedSlot = bIsSecondary ? GripInfo.SecondaryGripInfo.SecondarySlotName : GripInfo.SlotName;
		const bool bImplementsInterface = GripInfo.GrippedObject->GetClass()->ImplementsInterface(UTVRHandSocketInterface::StaticClass());
		UHandSocketComponent* HandSocket = bImplementsInterface ?
			ITVRHandSocketInterface::Execute_GetHandSocket(GripInfo.GrippedObject, GrippedSlot) :
			UHandSocketComponent::GetHandSocketComponentFromObject(GripInfo.GrippedObject, GrippedSlot);
		
		if(HandSocket)
		{
			const FTransform TargetRelativeTransform = HandSocket->GetMeshRelativeTransform(HandType == EControllerHand::Right);
			const FTransform HandSocketParentTF = HandSocket->GetAttachParent()->GetComponentTransform();
			TargetMeshTransform = TargetRelativeTransform * HandSocketParentTF;
			bUseTargetMeshTransform = true;
			if(HandSocket->GetBlendedPoseSnapShot(CustomPose, GetSkeletalMeshComponent()))
			{
				HandSocketComponent = HandSocket;
				bHasCustomAnimation = true;
				bCustomAnimIsSnapShot = true;
				return;
			}
			bHasCustomAnimation = false;
			HandSocketComponent = nullptr;
			return;
		}
	}
	HandSocketComponent = nullptr;
	bUseTargetMeshTransform = false;
	bHasCustomAnimation = false;

}

void ATVRGraspingHand::SetFingerCollisions()
{
	BPSetFingerCollisions();
	ClearFingers();
}

void ATVRGraspingHand::ClearFingers()
{
	for(auto It = FingerCollisionZones.CreateConstIterator(); It; ++It)
	{
		ETriggerIndices Key = It.Key();
		FingersBlocked.Add(Key, false);
		FingersOverlapping.Add(Key, false);
		FingerFlex.Add(Key, 0.f);
	}
}

void ATVRGraspingHand::SetPhysicalRelativeTransform()
{
	// const bool bHasNegativeScale = GetSkeletalMeshComponent()->GetComponentScale().GetMin() < 0.f;
	// if(bHasNegativeScale)
	// {
	// 	const FTransform ReverseTransform = FTransform(
	// 		FRotator(0.f, 0.f, -180.f),
	// 		FVector::ZeroVector,
	// 		FVector::OneVector
	// 	);
	// 	const FTransform NewTransform = BaseRelativeTransform * ReverseTransform;
	// 	GetSkeletalMeshComponent()->SetRelativeTransform(NewTransform, false, nullptr, ETeleportType::TeleportPhysics);
	// }
	// else
	{
		GetSkeletalMeshComponent()->SetRelativeTransform(BaseRelativeTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
}
