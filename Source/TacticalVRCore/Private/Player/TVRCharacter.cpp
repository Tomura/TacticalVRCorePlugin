// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/TVRCharacter.h"

#include "Settings/TVRCoreGameplaySettings.h"
#include "GameplayTags.h"
#include "TacticalCollisionProfiles.h"

#include "Libraries/TVRFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/TVRGunBase.h"
#include "Components/TVRClimbableCapsuleComponent.h"
#include "Player/TVRCharacterMovementComponent.h"
#include "TacticalTraceChannels.h"
#include "Player/TVRGraspingHand.h"
#include "Player/PauseMenuActor.h"
#include "Components/SphereComponent.h"
#include "Components/TVRHoverInputVolume.h"
#include "Components/WidgetInteractionComponent.h"
#include "Player/TVRPlayerController.h"
#include "Weapon/TVRMagazine.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
// #include "Net/UnrealNetwork.h" // todo: include once networking is relevant


#define DONT_USE_TIMER_TIME 0.01f

ATVRCharacter::ATVRCharacter(const FObjectInitializer& OI) 
    : Super(OI
        .SetDefaultSubobjectClass<UTVRCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)
    )
{
    WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(FName("WidgetInteraction"));
    WidgetInteraction->bAutoActivate = false;
    WidgetInteraction->TraceChannel = ECC_WidgetTraceChannel;
    WidgetInteraction->SetupAttachment(RightMotionController);

    GrabSphereLeft = CreateDefaultSubobject<USphereComponent>(FName("GrabSphereLeft"));
    GrabSphereLeft->SetupAttachment(LeftMotionController);
    GrabSphereLeft->InitSphereRadius(4.f);
    GrabSphereLeft->SetRelativeLocation(FVector(2.f, 0.f, -2.5f));
    GrabSphereLeft->SetCollisionProfileName(COLLISION_NO_COLLISION); // maybe use overlap all

    GrabSphereRight = CreateDefaultSubobject<USphereComponent>(FName("GrabSphereRight"));
    GrabSphereRight->SetupAttachment(RightMotionController);
    GrabSphereRight->InitSphereRadius(4.f);
    GrabSphereRight->SetRelativeLocation(FVector(2.f, 0.f, -2.5f));
    GrabSphereRight->SetCollisionProfileName(COLLISION_NO_COLLISION); // maybe use overlap 

    HandMeshRight = CreateDefaultSubobject<USkeletalMeshComponent>(FName("RightHandMesh"));
    HandMeshRight->SetupAttachment(RightMotionController);
    HandMeshRight->SetRelativeLocation(FVector(-12.785f, -0.028f, -1.789));
    HandMeshRight->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
    
    HandMeshLeft = CreateDefaultSubobject<USkeletalMeshComponent>(FName("LeftHandMesh"));
    HandMeshLeft->SetupAttachment(LeftMotionController);
    HandMeshLeft->SetRelativeLocation(FVector(-12.785f, -0.028f, -1.789));
    HandMeshLeft->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));
		
	LeftHandGripComponent = nullptr;
	RightHandGripComponent = nullptr;
    
    bAlreadyPossessed = false;
    
    RightGraspingHandClass = nullptr;
    RightGraspingHand = nullptr;
    LeftGraspingHand = nullptr;

	PendingTurn = 0.f;
	PrevTurnX = 0.f;
	bBlockTurn = false;
	
	VelocitySampleSize = 30;
	PeakVelocityLeft = FBPLowPassPeakFilter();
	PeakVelocityRight = FBPLowPassPeakFilter();
	PeakVelocityLeft.VelocitySamples = VelocitySampleSize;
	PeakVelocityRight.VelocitySamples = VelocitySampleSize;

	bSampleGripVelocity = true;
	bUseControllerVelocityOnRelease = false;
	bScaleThrowingVelocityByMass = false;
	MaxThrowingGripMass = 50.f;
	MinThrowingMassScale = 0.3f;
	bLimitMaxThrowingVelocity = true;
	MaxThrowingVelocity = 1000.f;
    bIsInMenu = false;
    MenuOpenTime = 0.5f;

	SprintMinWeaponDown = 0.6f;
	SprintMaxWeaponDown = 0.85f;
	SprintMinAim = 0.6f;
	SprintMaxAim = 0.85f;

	bIsActionAPressed_L = false;
	bIsActionAPressed_R = false;
	bIsActionBPressed_L = false;
	bIsActionBPressed_R = false;

	GrabHysteresisLeft = FTVRHysteresisValue(0.2f, 0.5f);
	GrabHysteresisRight = FTVRHysteresisValue(0.2f, 0.5f);
}

void ATVRCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATVRCharacter, bIsInMenu, COND_SkipOwner); // Owner is mostly authoritive here. Server can force it off using a client function if necessary
	DOREPLIFETIME_CONDITION(ATVRCharacter, RightControllerOffset, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ATVRCharacter, LeftControllerOffset, COND_SkipOwner);
}


void ATVRCharacter::PostInitProperties()
{
	Super::PostInitProperties();
	PeakVelocityLeft.VelocitySamples = VelocitySampleSize;
	PeakVelocityRight.VelocitySamples = VelocitySampleSize;
}

void ATVRCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	LeftMotionController->OnGrippedObject.AddDynamic(this, &ATVRCharacter::OnGrippedObjectLeft);
	RightMotionController->OnGrippedObject.AddDynamic(this, &ATVRCharacter::OnGrippedObjectRight);

	LeftMotionController->OnControllerProfileTransformChanged.AddDynamic(this, &ATVRCharacter::OnLeftControllerProfileChanged);
	RightMotionController->OnControllerProfileTransformChanged.AddDynamic(this, &ATVRCharacter::OnRightControllerProfileChanged);
}

void ATVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(IsLocallyControlled())
	{
		HandleTurning(DeltaTime);
		HandleMovement(DeltaTime);
		SampleGripVelocities();
	    TickWidgetInteraction(DeltaTime);

		if(RightGraspingHand && LeftGraspingHand)
		{
			RightGraspingHand->TriggerPress = TriggerAxisR;
			LeftGraspingHand->TriggerPress = TriggerAxisL;
		}
	}
}

void ATVRCharacter::BeginPlay()
{
	Super::BeginPlay();
    if(GetLocalRole() != ROLE_Authority)
    {
        SetupHands();
    }
}

void ATVRCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if(bAlreadyPossessed)
    {
        UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
    }
    else
    {
        ClientPossessed();
        bAlreadyPossessed = true;
        SetupHands();
    }
}

void ATVRCharacter::ClientPossessed_Implementation()
{
    // todo: refactor. this is hard to understand.
    if(UVRExpansionFunctionLibrary::IsInVREditorPreviewOrGame())
    {
        const EBPHMDDeviceType TempHeadsetType = UVRExpansionFunctionLibrary::GetHMDType();
        HeadsetType = TempHeadsetType;
    	UVRGlobalSettings::LoadControllerProfileByName(FName("Default"), true);
    }
}

void ATVRCharacter::SetupHands()
{
    if(GrabSphereRight)
    {
        RightMotionController->SetCustomPivotComponent(GrabSphereRight);
    }
    if(GrabSphereLeft)
    {
        LeftMotionController->SetCustomPivotComponent(GrabSphereLeft);
    }
    
    if(IsLocallyControlled())
    {
        UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
        UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::SingleEyeCroppedToFill);
    }
    
    SpawnGraspingHands();
}

void ATVRCharacter::RepositionHands(bool bIsRightHand, const FTransform& NewTransform)
{
	// would change relative transform of display component of motion controller to newtransform
}

void ATVRCharacter::OnRightControllerProfileChanged(const FTransform& NewTransformForComps,
	const FTransform& NewProfileTransform)
{
	RepositionHands(true, NewTransformForComps);
	ServerSetControllerProfile(true, NewTransformForComps);
}

void ATVRCharacter::OnLeftControllerProfileChanged(const FTransform& NewTransformForComps,
	const FTransform& NewProfileTransform)
{
	RepositionHands(false, NewTransformForComps);
	ServerSetControllerProfile(false, NewTransformForComps);
}

void ATVRCharacter::ServerSetControllerProfile_Implementation(bool bIsRightHand, FTransform_NetQuantize NewTransform)
{
	if(bIsRightHand)
	{
		RightControllerOffset = NewTransform;
		RepositionHands(true, NewTransform);
	}
	else
	{
		LeftControllerOffset = NewTransform;
		RepositionHands(false, NewTransform);
	}
}

void ATVRCharacter::OnRepRightControllerOffset(FTransform_NetQuantize NewValue)
{
	RepositionHands(true, NewValue);
}

void ATVRCharacter::OnRepLeftControllerOffset(FTransform_NetQuantize NewValue)
{
	RepositionHands(false, NewValue);
}


void ATVRCharacter::TickWidgetInteraction(float DeltaTime)
{
    if(PauseMenuActor != nullptr)
    {
        const FVector TraceStart = WidgetInteraction->GetComponentLocation();
        const FVector TraceEnd = TraceStart + WidgetInteraction->GetForwardVector() * 500.f;
        
        GetWorld()->LineTraceSingleByChannel(WidgetHitResult, TraceStart, TraceEnd, ECC_WidgetTraceChannel);
        if(WidgetHitResult.bBlockingHit && WidgetHitResult.Component == PauseMenuActor->GetCurrentlyActiveWidget())
        {
            WidgetInteraction->SetCustomHitResult(WidgetHitResult);
        }
        else
        {
            const FHitResult EmptyHitResult = FHitResult(ForceInit);
            WidgetInteraction->SetCustomHitResult(EmptyHitResult);
        }
    }
}


bool ATVRCharacter::CanMove() const
{
	// todo: return false if climbing (ladder)
	// todo return false if bMovementDisabled
	if(VRMovementReference != nullptr && VRMovementReference->IsClimbing())
	{
	    return false;
	}
	return !bIsInMenu;
}
bool ATVRCharacter::CanTurn() const
{
	// todo: return false if climbing (ladder)
	// todo return false if bMovementDisabled
	return !bIsInMenu;
}

void ATVRCharacter::GetMovementAxes(FVector& OutAxisForward, FVector& OutAxisRight) const
{
	const UTVRCoreGameplaySettings* SettingsCDO = GetDefault<UTVRCoreGameplaySettings>();
	switch(SettingsCDO->LocomotionStyle)
	{
	case ELocomotionStyle::LOC_ContiniousHand:
		// this should be sufficient.
		// The BP example projects the vector onto a N-(0,0,1) plane, and then normalizes.
		// This is mostly overkill, since it should eliminate the Z coordinate like this implementation
		// todo: Sometimes right hand could be assined for movement. Make this code respect the setting
		OutAxisForward = LeftMotionController->GetForwardVector().GetSafeNormal2D();
		OutAxisRight = LeftMotionController->GetRightVector().GetSafeNormal2D();
		break;
	case ELocomotionStyle::LOC_ContiniousHMD:
		OutAxisForward = GetVRForwardVector();
		OutAxisRight = GetVRRightVector();
	}
}

UGripMotionControllerComponent* ATVRCharacter::GetControllerHand(EControllerHand HandType) const
{
	switch (HandType)
	{
	case EControllerHand::Left:
		return LeftMotionController;
	case EControllerHand::Right:
		return RightMotionController;
	default: break;
	}
	return nullptr;
}

UGripMotionControllerComponent* ATVRCharacter::GetOtherControllerHand(EControllerHand HandType) const
{
	switch (HandType)
	{
	case EControllerHand::Left:
        return GetControllerHand(EControllerHand::Right);
	case EControllerHand::Right:
        return GetControllerHand(EControllerHand::Left);
	default: break;
	}
	return nullptr;
}

UGripMotionControllerComponent* ATVRCharacter::GetOtherControllerHand(UGripMotionControllerComponent* InHand) const
{
    if(InHand == RightMotionController)
    {
        return LeftMotionController;
    }
    if(InHand == LeftMotionController)
    {
        return RightMotionController;
    }
    return nullptr;
}

USphereComponent* ATVRCharacter::GetGrabSphere(UGripMotionControllerComponent* InHand) const
{
    if(InHand == RightMotionController)
    {
        return GetRightGrabSphere();
    }
    if(InHand == LeftMotionController)
    {
        return GetLeftGrabSphere();
    }
    return nullptr;
}

float ATVRCharacter::GetSprintStrength() const
{
	ATVRGunBase* myGun = false;
	TArray<AActor*> GrippedActors;
	RightMotionController->GetGrippedActors(GrippedActors);
	LeftMotionController->GetGrippedActors(GrippedActors);
	for(AActor* TestActor : GrippedActors)
	{
		if(ATVRGunBase* Gun = Cast<ATVRGunBase>(TestActor))
		{
			myGun = Gun;
			break;
		}
	}
	const FVector ViewDir = VRReplicatedCamera->GetComponentRotation().Vector();
	// we want to sprint forward
	const FVector MoveVel = GetVelocity().GetSafeNormal();
	const float MinSprint = 0.5f;
	const float MoveVelStrength = (FMath::Clamp( MoveVel | ViewDir, MinSprint, 1.f) - MinSprint) / (1.f - MinSprint);
	
	if(myGun != nullptr)
	{
		const FVector GunDir = myGun->GetActorRightVector();
		const float AimStrength = FMath::Clamp(
			(1.f - (GunDir | ViewDir) - SprintMinAim) / (SprintMaxAim - SprintMinAim),
			0.f, 1.f);
		
		const float RightVecZ = -1.f*GunDir.Z;
		const float WeaponDownStrength = FMath::Clamp(
			(RightVecZ - SprintMinWeaponDown) / (SprintMaxWeaponDown - SprintMinWeaponDown),
			0.f, 1.f);
		
		
		return FMath::Min(WeaponDownStrength, AimStrength) * MoveVelStrength;	
	}
	return MoveVelStrength;
}

ATVRGraspingHand* ATVRCharacter::GetLeftGraspingHand() const
{
	return LeftGraspingHand;
}

ATVRGraspingHand* ATVRCharacter::GetRightGraspingHand() const
{
	return RightGraspingHand;
}

ATVRGraspingHand* ATVRCharacter::GetGraspingHand(EControllerHand HandType) const
{
	switch(HandType)
	{
		case EControllerHand::Left:
			return GetLeftGraspingHand();
		case EControllerHand::Right:
			return GetRightGraspingHand();
		default:
			return nullptr;
	}
}

ATVRGraspingHand* ATVRCharacter::GetGraspingHand(UGripMotionControllerComponent* TestHand) const
{
	if(TestHand == LeftMotionController)
	{
		return GetLeftGraspingHand();
	}
	if(TestHand == RightMotionController)
	{
		return GetRightGraspingHand();
	}
	return nullptr;
}

void ATVRCharacter::HandleMovement(float DeltaTime)
{
	if(!CanMove())
	{
		return;
	}

	UTVRCharacterMovementComponent* MyMoveComp = Cast<UTVRCharacterMovementComponent>(VRMovementReference);
	if(MyMoveComp)
	{
		const float DefaultSpeed = GetDefault<ATVRCharacter>(GetClass())->VRMovementReference->MaxWalkSpeed;
		MyMoveComp->MaxWalkSpeed = FMath::Lerp(DefaultSpeed, DefaultSpeed*1.6f, GetSprintStrength());
	}
	
	const float AxisRight = AxisMove.X;
	const float AxisForward = AxisMove.Y;
	if(AxisForward != 0.f || AxisRight != 0.f)
	{
		FVector MoveAxisForward = FVector::ZeroVector;
		FVector MoveAxisRight = FVector::ZeroVector;
		GetMovementAxes(MoveAxisForward,MoveAxisRight);
		AddMovementInput(MoveAxisForward, AxisForward, false);
		AddMovementInput(MoveAxisRight, AxisRight, false);
	}

}

void ATVRCharacter::HandleTurning(float DeltaTime)
{
	if(IsLocallyControlled() && CanTurn())
	{
		const UTVRCoreGameplaySettings* SettingsCDO = GetDefault<UTVRCoreGameplaySettings>();
		const float AxisValue = AxisTurn.X;
		switch (SettingsCDO->TurnStyle)
		{
		case ERotationStyle::ROT_Continuous:
			{
				const float TurnSpeed = SettingsCDO->TurnSpeed;
				VRMovementReference->PerformMoveAction_SnapTurn(AxisValue * TurnSpeed * DeltaTime, EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn);
			}
			break;
		case ERotationStyle::ROT_ContinuousSnap:
			if(PendingTurn != 0.f) // works in this case
			{			
				const float TurnSpeed = SettingsCDO->SnapTurnSpeed;
				const float TurnSign = FMath::Sign(PendingTurn);
				float DeltaTurn = TurnSign * TurnSpeed * DeltaTime;
				if(FMath::Abs(DeltaTurn) >= FMath::Abs(PendingTurn))
				{
					DeltaTurn = PendingTurn;
					PendingTurn = 0.f;
				}
				else
				{
					PendingTurn -= DeltaTurn;
				}
				VRMovementReference->PerformMoveAction_SnapTurn(DeltaTurn, EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn);
			}
			//break;
		case ERotationStyle::ROT_Snap:
			{
				const auto GameplaySettings = UTVRCoreGameplaySettings::Get();
				const float TurnThreshold = GameplaySettings->SnapTurnHisteresis.GetUpperBoundValue();
				const float TurnHysteresis = GameplaySettings->SnapTurnHisteresis.GetLowerBoundValue();

				// GameplaySettings->SnapTurnHisteresis.
				
				if(!bBlockTurn)
				{
					if(AxisValue > TurnThreshold && PrevTurnX < TurnThreshold)
					{
						StartSnapTurn(ETurnDirection::Right);
						bBlockTurn = true;
					}
                    else if(AxisValue < -TurnThreshold && PrevTurnX > -TurnThreshold)
					{
						StartSnapTurn(ETurnDirection::Left);
						bBlockTurn = true;
					}
				}
				else if(FMath::Abs(AxisValue) < TurnHysteresis)
				{
					bBlockTurn = false;
				}				
				PrevTurnX = AxisValue;
			}
			break;
		default:
			break;
		}
	}
}

void ATVRCharacter::SnapTurnRight()
{
	StartSnapTurn(ETurnDirection::Right);
}

void ATVRCharacter::SnapTurnLeft()
{
	StartSnapTurn(ETurnDirection::Left);
}

void ATVRCharacter::OnAxisTurnX(float Value)
{
	AxisTurn.X = FMath::Clamp(Value, -1.f, 1.f);
}

void ATVRCharacter::OnAxisMoveX(float Value)
{
	AxisMove.X = FMath::Clamp(Value, -1.f, 1.f);
}

void ATVRCharacter::OnAxisMoveY(float Value)
{
	AxisMove.Y = FMath::Clamp(Value, -1.f, 1.f);
}

void ATVRCharacter::OnTriggerAxisL(float Value)
{
	TriggerAxisL = FMath::Clamp(Value, 0.f, 1.f);
}

void ATVRCharacter::OnTriggerAxisR(float Value)
{
	TriggerAxisR = FMath::Clamp(Value, 0.f, 1.f);
}

void ATVRCharacter::StartSnapTurn(ETurnDirection TurnDir)
{
	if(!CanTurn())
	{ // do not turn if we aren't allowed
		return;
	}
	const UTVRCoreGameplaySettings* SettingsCDO = GetDefault<UTVRCoreGameplaySettings>();
	if(SettingsCDO->TurnStyle == ERotationStyle::ROT_Snap || SettingsCDO->TurnStyle == ERotationStyle::ROT_ContinuousSnap)
	{
		// turn is on cooldown. Don't do anything
		if(SettingsCDO->TurnStyle == ERotationStyle::ROT_Snap  && GetWorldTimerManager().IsTimerActive(SnapTurnTimerHandle))
		{
			return;
		}
		// ignore when we are still turning, otherwise we can accumulate too much
		if(PendingTurn > KINDA_SMALL_NUMBER)
		{
			return;
		}

		// Pending Turn that is consumed during Tick(). This applies to both snap and continuous snap
		const float TurnAngle = SettingsCDO->TurnIncrement * (TurnDir == ETurnDirection::Right ? 1.f : -1.f);
		PendingTurn = TurnAngle;

		// Snap Turn specific fading logic
		if(SettingsCDO->TurnStyle == ERotationStyle::ROT_Snap)
		{
			if(const auto PC = Cast<APlayerController>(Controller))
			{
				const float FadeDuration = SettingsCDO->SnapTurnFadeDuration * 0.4f;
				if(FadeDuration > DONT_USE_TIMER_TIME) // only use timers if we actually need to wait
				{
					PC->PlayerCameraManager->StartCameraFade(0.f,
	                    1.f,
	                    FadeDuration,
	                    FLinearColor::Black,
	                    false,
	                    true
	                );
					GetWorldTimerManager().SetTimer(SnapTurnTimerHandle, this, &ATVRCharacter::PerformSnapTurn, FadeDuration, false);
				}
				else
				{
					PerformSnapTurn();
				}
			}
		}

	}
}

void ATVRCharacter::PerformSnapTurn()
{
	// This movement action is replicated
	VRMovementReference->PerformMoveAction_SnapTurn(PendingTurn, EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn);
	PendingTurn = 0.f;
	const UTVRCoreGameplaySettings* SettingsCDO = GetDefault<UTVRCoreGameplaySettings>();
	const float FadeDuration = SettingsCDO->SnapTurnFadeDuration * 0.2f;	
	if(FadeDuration > DONT_USE_TIMER_TIME)
	{
		GetWorldTimerManager().SetTimer(SnapTurnTimerHandle, this, &ATVRCharacter::FinishSnapTurn, FadeDuration, false);
	} else
	{
		FinishSnapTurn();
	}
}

void ATVRCharacter::FinishSnapTurn()
{	
	const UTVRCoreGameplaySettings* SettingsCDO = GetDefault<UTVRCoreGameplaySettings>();
	const float FadeDuration = SettingsCDO->SnapTurnFadeDuration * 0.4f;
	if(const auto PC = Cast<APlayerController>(Controller))
	{
		PC->PlayerCameraManager->StartCameraFade(1.f,
            0.f,
            FadeDuration,
            FLinearColor::Black,
            false,
            false
        );
	}
}

void ATVRCharacter::GripLeftPressed()
{
	if(LeftMotionController && RightMotionController)
	{
		
	}
}

void ATVRCharacter::GripLeftReleased()
{
	if(LeftMotionController && RightMotionController)
	{
	}
}



// todo: replace this with a static function in a static function library
bool ATVRCharacter::IsLocalGrip(EGripMovementReplicationSettings RepType) const
{
	return (RepType == EGripMovementReplicationSettings::ClientSide_Authoritive) || (RepType == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep);
}

bool ATVRCharacter::TryGrip(UGripMotionControllerComponent* Hand, bool bIsLargeGrip)
{
	if(Hand->HasGrippedObjects())
	{
		// todo: in this case we might want to drop, if the setting is toggle
		return false;
	}
	FBPActorGripInformation GripInfo;
    UGripMotionControllerComponent* OtherHand = GetOtherControllerHand(Hand);
    UPrimitiveComponent* GrabSphere = GetGrabSphere(Hand);
	// Check for secondary attachment
	if(OtherHand->GetIsSecondaryAttachment(Hand, GripInfo))
	{
		return false;
	}

	// todo: check secondary
	TArray<FHitResult> Hits;
	if(TraceGrips(Hits, Hand, GrabSphere))
	{
		FHitResult* BestHit = nullptr;
		uint8 BestPriority = 0;
		UObject* BestObject = nullptr;
		FVector BestLoc;
		for(FHitResult& Hit : Hits)
		{
			UObject* GripInterface = nullptr;
			
			const bool bIsGripInterface = Hit.GetComponent()->Implements<UVRGripInterface>();
			if(bIsGripInterface)
			{
				GripInterface = Hit.GetComponent();
			}
			else
			{
				AActor* GripOwner = Hit.GetComponent()->GetOwner();
				if(GripOwner == nullptr)
				{
					continue;
				}

				if(const auto GrippedWPNAttachment = Cast<ATVRWeaponAttachment>(GripOwner))
				{
					if(const auto Gun = GrippedWPNAttachment->GetGunOwner())
					{
						GripOwner = Gun;
					}
				}
				
				const bool bOwnerImplementInterface = GripOwner->Implements<UVRGripInterface>();
				if(bOwnerImplementInterface)
				{
					GripInterface = GripOwner;
				}
			}
			if(!IsGripValid(GripInterface, bIsLargeGrip, Hand))
			{
				continue;
			}
			// todo: might insert some special code for secondary gripping
			const FBPAdvGripSettings Settings = IVRGripInterface::Execute_AdvancedGripSettings(GripInterface);
			int32 DeltaPrio = BestObject == nullptr ? 1 : Settings.GripPriority - BestPriority;

			if(BestObject != nullptr)
			{
				FBPActorGripInformation OtherGripInfo;
				EBPVRResultSwitch Result;
				OtherHand->GetGripByObject(OtherGripInfo, BestObject, Result);
				if(Result == EBPVRResultSwitch::OnSucceeded)
				{
					if(OtherGripInfo.bIsSlotGrip) // todo maybe check if there is a possibility for secondary slot grip
					{
						bool bHasSecondary = false;
						FTransform SecondaryTF;
						FName SlotName;
						IVRGripInterface::Execute_ClosestGripSlotInRange(BestObject, Hand->GetComponentLocation(), true, bHasSecondary, SecondaryTF, SlotName, Hand, NAME_None);
						if(!bHasSecondary)
						{
							DeltaPrio = 1; // we do not wanna steal is possible							
						}
					}
				}
			}
			
			if(DeltaPrio == 0) // if there is equal priority we need to still resolve this
			{
				// We check the distance between trace start center of grip sphere, to the impact of the sweep
				const FVector TraceStart = Hit.TraceStart;
				const float DistA = (Hit.TraceStart - Hit.ImpactPoint).SizeSquared();
				const float DistBest = (Hit.TraceStart - BestHit->ImpactPoint).SizeSquared();
				if(DistA < DistBest)
				{
					DeltaPrio = 1;
				}
			}
			
			if(IsGripValid(GripInterface, bIsLargeGrip, Hand) && DeltaPrio > 0)
			{
				BestPriority = Settings.GripPriority;
				BestObject = GripInterface;
				BestHit = &Hit;
			}
		}

		if(BestObject != nullptr && BestHit != nullptr)
		{
			return AttemptToGripObject(BestObject, Hand, OtherHand, *BestHit);
		}	
	}
	return false;
}

bool ATVRCharacter::TryDrop(UGripMotionControllerComponent* Hand, bool bIsLarge)
{
    UGripMotionControllerComponent* OtherHand = GetOtherControllerHand(Hand);
	if(Hand->HasGrippedObjects())
	{
		TArray<FBPActorGripInformation> AllGrips;
		Hand->GetAllGrips(AllGrips);
		for(int i = (AllGrips.Num()-1); i >= 0; i--)
		{
			FBPActorGripInformation& Grip = AllGrips[i];
			if (UTVRFunctionLibrary::IsObjectGripType(Grip.GrippedObject, bIsLarge))
			{
				TryDropObject(Hand, Grip);
				return true;
			}
		}
	}

	// if there are no primary grips, remove secondary grips
	FBPActorGripInformation GripInfo;
	if(OtherHand->GetIsSecondaryAttachment(Hand, GripInfo))
	{
		if (UTVRFunctionLibrary::IsObjectGripType(GripInfo.GrippedObject, bIsLarge))
		{
			if(IsLocalGrip(GripInfo.GripMovementReplicationSetting))
			{
				LocalRemoveSecondaryAttachmentGrip(OtherHand, GripInfo.GrippedObject);
			}
			else
			{
				ServerRemoveSecondaryAttachmentGrip(OtherHand, GripInfo.GrippedObject);
			}
			return true;
		}
	}
	
	return false;
}

bool ATVRCharacter::TraceGrips(TArray<FHitResult>& OutHits, UGripMotionControllerComponent* Hand, UPrimitiveComponent* OverlapComp)
{	// Trace for object first
	constexpr float TraceLength = 0.01;
	const FVector TraceStart = OverlapComp->Bounds.Origin - OverlapComp->GetForwardVector() * TraceLength;
	const FVector TraceStop = OverlapComp->Bounds.Origin + OverlapComp->GetForwardVector() * TraceLength;
	const float SweepRadius = OverlapComp->Bounds.SphereRadius;
	TArray<AActor*> IgnoreActors;
	Hand->GetGrippedActors(IgnoreActors);
	FCollisionQueryParams QueryParams(FName("TraceGrip"), false);
	QueryParams.AddIgnoredActors(IgnoreActors);
	QueryParams.AddIgnoredActor(this);
	const bool bHitSomething = GetWorld()->SweepMultiByChannel(OutHits, TraceStart, TraceStop, FQuat(), ECC_VRTraceChannel, 
    FCollisionShape::MakeSphere(SweepRadius), QueryParams);
	return bHitSomething;
}

bool ATVRCharacter::IsGripValid(UObject* ObjectToGrip, bool bIsLargeGrip, UGripMotionControllerComponent* GripInitiator) const
{
	if(ObjectToGrip == nullptr || !ObjectToGrip->Implements<UVRGripInterface>())
	{
		return false;
	}
	if(IVRGripInterface::Execute_DenyGripping(ObjectToGrip, GripInitiator))
	{
		return false;;
	}
	if(!UTVRFunctionLibrary::IsObjectGripType(ObjectToGrip, bIsLargeGrip))
	{
		return false;
	}
	return true;
}

bool ATVRCharacter::TryStartClimbing(UTVRClimbableCapsuleComponent* ClimbableComp,
    UGripMotionControllerComponent* GrippingHand)
{
    if(ClimbableComp == nullptr || GrippingHand == nullptr)
    {
        return  false;
    }

    UTVRCharacterMovementComponent* MyMoveComp = Cast<UTVRCharacterMovementComponent>(VRMovementReference);
    if(MyMoveComp)
    {
        if(MyMoveComp->IsClimbing())
        {
            EControllerHand HandType = EControllerHand::AnyHand;
            GrippingHand->GetHandType(HandType);
            UGripMotionControllerComponent* OtherHand = GetOtherControllerHand(HandType);

            if(MyMoveComp->IsClimbingHand(OtherHand))
            {
                OtherHand->DropObjectByInterface(MyMoveComp->CurrentClimbComp(), 0, FVector::ZeroVector,FVector::ZeroVector);
            }
        }
        
        FClimbInfo NewClimbInfo;
        NewClimbInfo.GripLocation = ClimbableComp->GetComponentTransform().InverseTransformPosition(GrippingHand->GetComponentLocation());
        NewClimbInfo.GrippedCapsule = ClimbableComp;
        NewClimbInfo.GrippingHand = GrippingHand;
        NewClimbInfo.bIsRelative = true;
        MyMoveComp->InitiateClimbing(NewClimbInfo);
        return true;
    }
    return false;
}

UTVRHoverInputVolume* ATVRCharacter::GetOverlappingHoverInputComp(USphereComponent* GrabSphere) const
{
	TArray<UPrimitiveComponent*> Overlaps;
	GrabSphere->GetOverlappingComponents(Overlaps);
	for(UPrimitiveComponent* TestComp : Overlaps)
	{
		if(UTVRHoverInputVolume* TestInputVol = Cast<UTVRHoverInputVolume>(TestComp))
		{
			if(TestInputVol->IsActive())
			{
				return TestInputVol;
			}
		}			
	}
	return nullptr;
}

void ATVRCharacter::OnGrabLargeLeft()
{
    TryGrip(LeftMotionController, true);
}

void ATVRCharacter::OnGrabLargeRight()
{
    TryGrip(RightMotionController, true);
}

void ATVRCharacter::OnStopGrabLargeLeft()
{
    UseHeldObject(LeftMotionController, false); // todo: replace with stop all use
    TryDrop(LeftMotionController, true);
}

void ATVRCharacter::OnStopGrabLargeRight()
{
    UseHeldObject(RightMotionController, false); // todo: replace with stop all use
    TryDrop(RightMotionController, true);
}

void ATVRCharacter::OnGrabAxisLeft(float Value)
{
}

void ATVRCharacter::OnGrabAxisRight(float Value)
{
}

void ATVRCharacter::OnUseOrGrabSmallLeft()
{    
    GrabOrUse(LeftMotionController);
}

void ATVRCharacter::OnUseOrGrabSmallRight()
{
    GrabOrUse(RightMotionController);
}

void ATVRCharacter::OnStopUseOrGrabSmallLeft()
{    
    StopGrabOrUse(LeftMotionController);
}

void ATVRCharacter::OnStopUseOrGrabSmallRight()
{
    StopGrabOrUse(RightMotionController);
}

void ATVRCharacter::OnTriggerTouchRight()
{
	if(RightGraspingHand)
	{
		RightGraspingHand->bIsTriggerTouched = true;
	}
}

void ATVRCharacter::OnTriggerReleaseRight()
{
	if(RightGraspingHand)
	{
		RightGraspingHand->bIsTriggerTouched = false;
	}
}

void ATVRCharacter::OnTriggerTouchLeft()
{
	if(LeftGraspingHand)
	{
		LeftGraspingHand->bIsTriggerTouched = true;
	}
}

void ATVRCharacter::OnTriggerReleaseLeft()
{
	if(LeftGraspingHand)
	{
		LeftGraspingHand->bIsTriggerTouched = false;
	}
}

void ATVRCharacter::GrabOrUse(UGripMotionControllerComponent* UsingHand)
{
    if(WidgetInteraction->GetAttachParent() != UsingHand)
    {
        WidgetInteraction->AttachToComponent(UsingHand, FAttachmentTransformRules::SnapToTargetIncludingScale);
    }
    else
    {
        if(WidgetInteraction->IsActive() && PauseMenuActor != nullptr)
        {
            if(WidgetHitResult.GetComponent() == PauseMenuActor->GetCurrentlyActiveWidget())
            {
                WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
                return;
            }
            if(WidgetHitResult.bBlockingHit && WidgetHitResult.GetComponent() != nullptr)
            {
                PauseMenuActor->FocusWidget(Cast<UWidgetComponent>(WidgetHitResult.GetComponent()));
                return;
            }
        }
    }
	
    if(!TryGrip(UsingHand, false))
    {
    	FBPActorGripInformation Grip;
    	if(UsingHand->HasGrippedObjects() || GetOtherControllerHand(UsingHand)->GetIsSecondaryAttachment(UsingHand, Grip))
    	{
			UseHeldObject(UsingHand, true);    		
    	}
    	else
    	{
    		if(USphereComponent* GrabSphere = GetGrabSphere(UsingHand))
    		{
    			UTVRHoverInputVolume* TestInputVol = GetOverlappingHoverInputComp(GrabSphere);
    			if(TestInputVol && TestInputVol->OnUsed(UsingHand))
    			{
    				return;
    			}
    		}
    	}
    }
}

void ATVRCharacter::StopGrabOrUse(UGripMotionControllerComponent* UsingHand)
{
    if(WidgetInteraction->IsActive() && WidgetInteraction->GetAttachParent() == UsingHand)
    {
        WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
    }
    else
    {
        UseHeldObject(UsingHand, false);
        TryDrop(UsingHand, false);
    }
}

void ATVRCharacter::UseHeldObject(UGripMotionControllerComponent* UsingHand, bool bUse)
{
    TArray<FBPActorGripInformation> AllGrips;
    UsingHand->GetAllGrips(AllGrips);
    for(FBPActorGripInformation& Grip : AllGrips)
    {
        if(Grip.GrippedObject != nullptr)
        {
            const bool bImplementsGripInterface = Grip.GrippedObject->Implements<UVRGripInterface>();
            if(bImplementsGripInterface)
            {
                if(bUse)
                {
                    IVRGripInterface::Execute_OnUsed(Grip.GrippedObject);
                }
                else
                {
                    IVRGripInterface::Execute_OnEndUsed(Grip.GrippedObject);
                }
            }
        }
    }

	UGripMotionControllerComponent* OtherHand = GetOtherControllerHand(UsingHand);
	FBPActorGripInformation GripInfo;
	if(OtherHand->GetIsSecondaryAttachment(UsingHand, GripInfo))
	{
		if(GripInfo.GrippedObject->Implements<UVRGripInterface>())
		{
			if(bUse)
			{
				IVRGripInterface::Execute_OnSecondaryUsed(GripInfo.GrippedObject);
			}
			else
			{
				IVRGripInterface::Execute_OnEndSecondaryUsed(GripInfo.GrippedObject);
			}
		}
	}
}

bool ATVRCharacter::AttemptToGripObject(UObject* ObjectToGrip, UGripMotionControllerComponent* Hand,
                                        UGripMotionControllerComponent* OtherHand, const FHitResult& Hit)
{
    // It is really a grip
	FTransform GripTransform;
	if(AActor* GripActor = Cast<AActor>(ObjectToGrip))
	{
		GripTransform = GripActor->GetActorTransform();
	}
	else if(UPrimitiveComponent* GripComp = Cast<UPrimitiveComponent>(ObjectToGrip))
	{
		GripTransform = GripComp->GetComponentTransform();
	}
	EControllerHand HandType;
	Hand->GetHandType(HandType);
	
	// first try secondary gripping
	if(AttemptToSecondaryGripObject(GripTransform, ObjectToGrip, Hand, Hit))
	{
		return true;
	}
	return AttemptToPrimaryGripObject(GripTransform, ObjectToGrip, Hand, Hit);
}

bool ATVRCharacter::AttemptToSecondaryGripObject(const FTransform& GripTransform, UObject* ObjectToGrip, UGripMotionControllerComponent* Hand, const FHitResult& Hit)
{	
	EControllerHand HandType;
	Hand->GetHandType(HandType);
	ESecondaryGripType SecondaryGripType = CanAttemptSecondaryGrabOnObject(ObjectToGrip);
	if(SecondaryGripType != ESecondaryGripType::SG_None)
	{
		bool bHadSlotInRange = false;
		FTransform SlotWorldTransform;
		FName SlotName;
		FName OverridePrefix = EName::NAME_None;
		IVRGripInterface::Execute_ClosestGripSlotInRange(ObjectToGrip, Hit.ImpactPoint, true, bHadSlotInRange, SlotWorldTransform,  SlotName, Hand, OverridePrefix);
		if(bHadSlotInRange)
		{
			// todo: add a case without slot for SG_Free, etc
			FTransform RelativeGripTransform = UGripMotionControllerComponent::ConvertToGripRelativeTransform(GripTransform, SlotWorldTransform);

			FGrabObjectInfo GripInfo;
			GripInfo.GripTransform = RelativeGripTransform;
			GripInfo.ObjectToGrip = ObjectToGrip;
			GripInfo.GripBoneName = EName::NAME_None;
			GripInfo.SlotName = SlotName;
			GripInfo.bIsSlotGrip = true;
			GripInfo.bIsSecondaryGrip = true;
			
			TryGrabObject(GripInfo, HandType);
			return true;
		}
		else if(false /* todo: for the case without a slot*/)
		{
			FTransform RelativeGripTransform = Hand->ConvertToGripRelativeTransform(GripTransform, Hand->GetComponentTransform());
		}
	}
	return false;
}

bool ATVRCharacter::AttemptToPrimaryGripObject(const FTransform& GripTransform, UObject* ObjectToGrip, UGripMotionControllerComponent* Hand, const FHitResult& Hit)
{
	EControllerHand HandType;
	Hand->GetHandType(HandType);
	
	bool bHadSlotInRange = false;
	FTransform SlotWorldTransform;
	FName SlotName;
	IVRGripInterface::Execute_ClosestGripSlotInRange(ObjectToGrip, Hit.ImpactPoint, false, bHadSlotInRange, SlotWorldTransform,  SlotName, Hand, EName::NAME_None);
	FTransform RelativeGripTransform;
	if(bHadSlotInRange)
	{
		RelativeGripTransform = GripTransform.GetRelativeTransform(SlotWorldTransform);
		RelativeGripTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
		// if(Hand->bOffsetByControllerProfile)
		// {
		// 	RelativeGripTransform=UVRGlobalSettings::AdjustTransformByControllerProfile(EName::NAME_None, RelativeGripTransform, HandType == EControllerHand::Right);
		// }			
	}
	else
	{
		const FTransform BaseTransform = Hit.BoneName.IsNone() ?
			GripTransform :
			RelativeGripTransform = Hit.GetComponent()->GetSocketTransform(Hit.BoneName, ERelativeTransformSpace::RTS_World);
		RelativeGripTransform = Hand->ConvertToControllerRelativeTransform(BaseTransform);
	}
	FGrabObjectInfo GripInfo;
	GripInfo.ObjectToGrip = ObjectToGrip;
	GripInfo.bIsSlotGrip = bHadSlotInRange;
	GripInfo.bIsSecondaryGrip = false;
	GripInfo.GripBoneName = bHadSlotInRange ? Hit.BoneName : EName::NAME_None;
	GripInfo.SlotName = SlotName;
	GripInfo.GripTransform = RelativeGripTransform;
	TryGrabObject(GripInfo, HandType);
	return true;
}

ESecondaryGripType ATVRCharacter::CanAttemptSecondaryGrabOnObject(UObject* ObjectToCheck) const
{
	if(ObjectToCheck->Implements<UVRGripInterface>())
	{
		TArray<FBPGripPair> HoldingControllers;
		bool bIsHeld = false;
		IVRGripInterface::Execute_IsHeld(ObjectToCheck, HoldingControllers, bIsHeld);
		if(bIsHeld && (HoldingControllers[0].HoldingController->GetOwner() == this))
		{
			return IVRGripInterface::Execute_SecondaryGripType(ObjectToCheck);
		}
	}
	return ESecondaryGripType::SG_None;
}

bool ATVRCharacter::LocalRemoveSecondaryAttachmentGrip(UGripMotionControllerComponent* GrippingHand,
	UObject* ObjectToRemove)
{
	return GrippingHand->RemoveSecondaryAttachmentPoint(ObjectToRemove, 0.25f);	 
}

void ATVRCharacter::ServerRemoveSecondaryAttachmentGrip_Implementation(UGripMotionControllerComponent* GrippingHand,
	UObject* ObjectToRemove)
{
	LocalRemoveSecondaryAttachmentGrip(GrippingHand, ObjectToRemove);
}


void ATVRCharacter::TryGrabObject(const FGrabObjectInfo& GripInfo, EControllerHand HandType)
{
	if(GripInfo.ObjectToGrip != nullptr) // todo: why do we still call it when the gripped object is zero`?
	{
		const bool bIsGripInterface = GripInfo.ObjectToGrip->Implements<UVRGripInterface>();
		if(bIsGripInterface)
		{
			const EGripMovementReplicationSettings RepType = IVRGripInterface::Execute_GripMovementReplicationType(GripInfo.ObjectToGrip);
			if(IsLocalGrip(RepType))
			{
				LocalTryGrabObject(
					GripInfo,
				    GetControllerHand(HandType),
				    GetOtherControllerHand(HandType)
				);
				return;
			}
		}
		ServerTryGrabObject(GripInfo, HandType);
	}
}


bool ATVRCharacter::LocalTryGrabObject(const FGrabObjectInfo& GripInfo, UGripMotionControllerComponent* GrippingHand, UGripMotionControllerComponent* OtherHand)
{
	if(GripInfo.ObjectToGrip == nullptr)
	{
		return false;
	}
	const bool bIsGripInterface = GripInfo.ObjectToGrip->Implements<UVRGripInterface>();
	if(GrippingHand->GetIsObjectHeld(GripInfo.ObjectToGrip)) // Does the hand already hold the object?
	{
		// already holding it, so no need to do something
		return false;
	}

	if(OtherHand->GetIsObjectHeld(GripInfo.ObjectToGrip)) // Does the other hand hold the object?
	{
		if(!bIsGripInterface || !IVRGripInterface::Execute_AllowsMultipleGrips(GripInfo.ObjectToGrip))
		{
			if(!GripInfo.bIsSlotGrip || GripInfo.bIsSecondaryGrip)
			{
				if(TrySecondaryGripObject(GripInfo, GrippingHand, OtherHand))
				{
					return true;
				}
			}			
			OtherHand->DropObject(GripInfo.ObjectToGrip, 0, false, FVector::ZeroVector, FVector::ZeroVector);
		}
	}
	else
	{
		// now we are sure this one is free (or held by someone else?)
		if(GripInfo.bIsSecondaryGrip)
		{
			// gripping a free object as secondary makes no sense here. We want to grip it as primary
			// todo: this might need an warning during development, but not during production
			return false;
		}

		if(bIsGripInterface)
		{
			bool bIsHeld = false;
			TArray<FBPGripPair> HoldingControllers;
			IVRGripInterface::Execute_IsHeld(GripInfo.ObjectToGrip, HoldingControllers, bIsHeld);
			if(bIsHeld && !IVRGripInterface::Execute_AllowsMultipleGrips(GripInfo.ObjectToGrip))
			{
				// we cannot grip if a object is already held and does not want multiple grips
				return false;
			}
		}
	}

	if(bIsGripInterface)
	{
		return GrippingHand->GripObjectByInterface(
			GripInfo.ObjectToGrip,
			GripInfo.GripTransform, true,
			GripInfo.GripBoneName, GripInfo.SlotName,
			GripInfo.bIsSlotGrip
		);
	}
	
	return GrippingHand->GripObject(
		GripInfo.ObjectToGrip,
		GripInfo.GripTransform, true,
		GripInfo.SlotName, GripInfo.GripBoneName,
		EGripCollisionType::InteractiveCollisionWithPhysics,
		EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping,
		EGripMovementReplicationSettings::ForceClientSideMovement,
		1500.f, 200.f, GripInfo.bIsSlotGrip
	);
}

void ATVRCharacter::ServerTryGrabObject_Implementation(const FGrabObjectInfo& GripInfo, EControllerHand HandType)
{
	LocalTryGrabObject(GripInfo, GetControllerHand(HandType), GetOtherControllerHand(HandType));
}

bool ATVRCharacter::TrySecondaryGripObject(const FGrabObjectInfo& GripInfo,	UGripMotionControllerComponent* GrippingHand, UGripMotionControllerComponent* OtherHand)
{
	if(CanSecondaryGripObject(GripInfo.ObjectToGrip, GripInfo.bIsSlotGrip))
	{
		return OtherHand->AddSecondaryAttachmentPoint(GripInfo.ObjectToGrip, GrippingHand, GripInfo.GripTransform, true, 0.25f, true, GripInfo.SlotName);
	}
	return false;
}

bool ATVRCharacter::CanSecondaryGripObject(UObject* ObjectToGrip, bool bHadSlot) const
{
	const bool bIsGripInterface = ObjectToGrip->Implements<UVRGripInterface>();
	if(!bIsGripInterface)
	{
		return false;
	}
	const ESecondaryGripType SecondaryType = IVRGripInterface::Execute_SecondaryGripType(ObjectToGrip);
	if(SecondaryType == ESecondaryGripType::SG_None)
	{
		return false;
	}
	if(bHadSlot)
	{
		return true;
	}
	switch (SecondaryType)
	{
		case ESecondaryGripType::SG_Free:
		case ESecondaryGripType::SG_Free_Retain:
		case ESecondaryGripType::SG_FreeWithScaling_Retain:
		case ESecondaryGripType::SG_ScalingOnly:
		case ESecondaryGripType::SG_Custom:
			return true;
		case ESecondaryGripType::SG_None:
	    case ESecondaryGripType::SG_SlotOnly:
		case ESecondaryGripType::SG_SlotOnly_Retain:
	    case ESecondaryGripType::SG_SlotOnlyWithScaling_Retain:
		default:
			break;
	}
	
	return false;
}

void ATVRCharacter::TryDropObject(UGripMotionControllerComponent* MotionController, const FBPActorGripInformation& Grip)
{
	FVector AngularVel, TransVel;
	MotionController->GetPhysicsVelocities(Grip, AngularVel, TransVel);
	GetThrowingVelocity(AngularVel, TransVel, MotionController, Grip, AngularVel, TransVel);
	
	if(Grip.GrippedObject != nullptr) // todo: why do we still call it when the gripped object is zero`?
	{
		const bool bIsGripInterface = Grip.GrippedObject->Implements<UVRGripInterface>();
		if(bIsGripInterface)
		{
			const EGripMovementReplicationSettings RepType = IVRGripInterface::Execute_GripMovementReplicationType(Grip.GrippedObject);
			if(IsLocalGrip(RepType))
			{
				LocalTryDropObject(MotionController, Grip, AngularVel, TransVel);
				return;
			}
		}
	}
	ServerTryDropObject(MotionController, AngularVel, TransVel, Grip.GripID);
}

bool ATVRCharacter::LocalTryDropObject(UGripMotionControllerComponent* MotionController, const FBPActorGripInformation& Grip, const FVector& AngleVel, const FVector& TransVel)
{
	if(Grip.GrippedObject != nullptr)
	{
		const bool bIsGripInterface = Grip.GrippedObject->Implements<UVRGripInterface>();
		if(bIsGripInterface)
		{
			USceneComponent* ParentToSocket = nullptr;
			FTransform_NetQuantize RelativeTransform;
			FName OptionalSocketName;
			const bool bShouldSocket = IVRGripInterface::Execute_RequestsSocketing(Grip.GrippedObject, ParentToSocket, OptionalSocketName, RelativeTransform);
			if(bShouldSocket)
			{
				return MotionController->DropAndSocketGrip(Grip, ParentToSocket, OptionalSocketName, RelativeTransform, true);
			}
			return MotionController->DropObjectByInterface(Grip.GrippedObject, 0, AngleVel, TransVel);
		}
	}
	return MotionController->DropGrip(Grip, true, AngleVel,TransVel);
}

void ATVRCharacter::ServerTryDropObject_Implementation(UGripMotionControllerComponent* MotionController, FVector_NetQuantize100 AngleVel, FVector_NetQuantize100 TransVel, uint8 GripHash)
{
	FBPActorGripInformation Grip;
	EBPVRResultSwitch Result;	
	MotionController->GetGripByID(Grip, GripHash, Result);
	if(Result == EBPVRResultSwitch::OnSucceeded)
	{
		LocalTryDropObject(MotionController, Grip, AngleVel, TransVel);
	}
}

void ATVRCharacter::GetThrowingVelocity(
	FVector& OutAngularVel,
	FVector& OutTransVel,
	UGripMotionControllerComponent* ThrowingController,
	const FBPActorGripInformation& Grip,
	const FVector& AngularVelocity,
	const FVector& TransVelocity
) const
{
	if(Grip.GrippedObject != nullptr) // maybe we can already assume here that gripped object is never null. Might replace with check or ensure
	{
		if(Grip.GrippedObject->Implements<UVRGripInterface>())
		{
			TArray<FBPGripPair> HoldingControllers;
			bool bIsHeld = false;
			IVRGripInterface::Execute_IsHeld(Grip.GrippedObject, HoldingControllers, bIsHeld);
			if(HoldingControllers.Num() > 1)
			{
				// another controller is holding this object, so return ZeroVector
				OutAngularVel = FVector::ZeroVector;
				OutTransVel = FVector::ZeroVector;
				return;
			}
		}

		FVector LocalVelocity; // = FVector::ZeroVector;
		if(bSampleGripVelocity)
		{
			EControllerHand HandType;
			ThrowingController->GetHandType(HandType);
			GetFilteredHandVelocity(LocalVelocity, HandType);
		}
		else if (bUseControllerVelocityOnRelease)
		{
			LocalVelocity = ThrowingController->GetComponentVelocity();
		}
		else
		{
			LocalVelocity = TransVelocity;
		}

		if(bScaleThrowingVelocityByMass)
		{
			float Mass = 0.f;
			ThrowingController->GetGripMass(Grip, Mass); // todo: this signature is shit. Compiler cannot optimize and we are allocating 8 + 4 bytes instead of just 4 + 4 max
			// we want to prevent a division by 0.
			// Since the scale would infinite clamped to 1. We resolve this by using 1
			const float MassScale  = (Mass > KINDA_SMALL_NUMBER) ?
				FMath::Clamp(MaxThrowingGripMass/Mass, MinThrowingMassScale, 1.f) : 1.f;
			LocalVelocity *= MassScale;
		}

		// Limit thowing velocity, when it is over the maximum.
		if(bLimitMaxThrowingVelocity && (LocalVelocity.SizeSquared() > (MaxThrowingVelocity*MaxThrowingVelocity)))
		{
			LocalVelocity = LocalVelocity.GetSafeNormal() * MaxThrowingVelocity;
		}
		OutAngularVel = AngularVelocity;
		OutTransVel = LocalVelocity;
	}
	// no component to throw
	OutAngularVel = FVector::ZeroVector;
	OutTransVel = FVector::ZeroVector;
}

FBPLowPassPeakFilter const* ATVRCharacter::GetHandVelocityFilter(EControllerHand HandType) const
{
	if(HandType == EControllerHand::Left)
	{
		return &PeakVelocityLeft;	
	}
	return &PeakVelocityRight;
}

void ATVRCharacter::GetFilteredHandVelocity(FVector& OutVelocity, EControllerHand HandType) const
{
	OutVelocity = GetHandVelocityFilter(HandType)->GetPeak();
}

void ATVRCharacter::SampleGripVelocities()
{
	if(bSampleGripVelocity)
	{
		SampleGripVelocity(LeftMotionController, PeakVelocityLeft);
		SampleGripVelocity(RightMotionController, PeakVelocityRight);
	}
}

void ATVRCharacter::SampleGripVelocity(UGripMotionControllerComponent* MotionController, FBPLowPassPeakFilter& Filter)
{
	TArray<FBPActorGripInformation> AllGrips;
	MotionController->GetAllGrips(AllGrips);
	if(AllGrips.Num() > 0)
	{
		FVector TransVel, AngularVel;
		MotionController->GetPhysicsVelocities(AllGrips[0], AngularVel, TransVel);
		Filter.AddSample(TransVel);
	}
	else // not necessary for a lot of applications, but we also sample the empty hand
	{
		Filter.AddSample(
			GetGraspingHand(MotionController)->GetSkeletalMeshComponent()->GetPhysicsLinearVelocity()
		);
	}
}

void ATVRCharacter::OnGrippedObjectLeft(const FBPActorGripInformation& Grip)
{
	if(bSampleGripVelocity)
	{
		PeakVelocityLeft.Reset();
	}
}

void ATVRCharacter::OnGrippedObjectRight(const FBPActorGripInformation& Grip)
{
	if(bSampleGripVelocity)
	{
		PeakVelocityRight.Reset();
	}
}

void ATVRCharacter::TogglePauseMenu(UGripMotionControllerComponent* UsingHand)
{
    OnStopUseOrGrabSmallLeft();
    OnStopUseOrGrabSmallRight();
    
	if(IsValid(PauseMenuActor) && PauseMenuActor != nullptr)
	{
		TryClosePauseMenu();
	}
	else if(CanOpenPauseMenu())
	{
	    if(GetWorldTimerManager().IsTimerActive(MenuOpenTimer))
	    {
	        GetWorldTimerManager().ClearTimer(MenuOpenTimer);
	    }
        GetWorldTimerManager().SetTimer(MenuOpenTimer, this, &ATVRCharacter::OpenPauseMenu, MenuOpenTime, false);
	}
	WidgetInteraction->AttachToComponent(UsingHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ATVRCharacter::ReleaseTogglePauseMenu(UGripMotionControllerComponent* UsingHand)
{
	if(GetWorldTimerManager().IsTimerActive(MenuOpenTimer) && WidgetInteraction->GetAttachParent() == UsingHand)
	{
		GetWorldTimerManager().ClearTimer(MenuOpenTimer);
	}
}

void ATVRCharacter::OpenPauseMenu()
{
    OnStopUseOrGrabSmallLeft();
    OnStopUseOrGrabSmallRight();
    
    if(GetWorldTimerManager().IsTimerActive(MenuOpenTimer))
    {
        GetWorldTimerManager().ClearTimer(MenuOpenTimer);
    }
    
    if(CanOpenPauseMenu())
    {
        TryOpenPauseMenu();
    }
}

bool ATVRCharacter::TryOpenPauseMenu()
{
    WidgetInteraction->Activate();
    bIsInMenu = true;
    OnOpenPauseMenu();
	return true;
}

void ATVRCharacter::TryClosePauseMenu()
{
    WidgetInteraction->Deactivate();
    bIsInMenu = false;
    OnClosePauseMenu();
}

bool ATVRCharacter::CanOpenPauseMenu()
{
	// Hand that pressed the pause menu button should not be holding something
	return true;
}

void ATVRCharacter::OnMagReleasePressed_Left()
{
	bIsActionAPressed_L = true;
	OnActionA_Pressed(LeftMotionController);
}

void ATVRCharacter::OnMagReleaseReleased_Left()
{
	bIsActionAPressed_L = false;
	OnActionA_Released(LeftMotionController);
}

void ATVRCharacter::OnMagReleasePressed_Right()
{
	bIsActionAPressed_R = true;
	OnActionA_Pressed(RightMotionController);
}

void ATVRCharacter::OnMagReleaseReleased_Right()
{
	bIsActionAPressed_R = false;
    OnActionA_Released(RightMotionController);
}


void ATVRCharacter::OnBoltReleasePressed_Left()
{
	bIsActionBPressed_L = true;
	OnActionB_Pressed(LeftMotionController);
}

void ATVRCharacter::OnBoltReleaseReleased_Left()
{
	bIsActionBPressed_L = false;
	OnActionB_Released(LeftMotionController);
}

void ATVRCharacter::OnBoltReleasePressed_Right()
{
	bIsActionBPressed_R = true;
	OnActionB_Pressed(RightMotionController);
}

void ATVRCharacter::OnBoltReleaseReleased_Right()
{
	bIsActionBPressed_R = false;
	OnActionB_Released(RightMotionController);
}

void ATVRCharacter::OnActionA_Pressed(UGripMotionControllerComponent* UsingHand)
{
	check(UsingHand);
	TArray<FBPActorGripInformation> AllGrips;
	FBPActorGripInformation GripInfo;
	
	UsingHand->GetAllGrips(AllGrips);
	
	const bool bIsPrimaryGrip = AllGrips.Num() > 0;
	const bool bIsSecondaryGrip = GetOtherControllerHand(UsingHand)->GetIsSecondaryAttachment(UsingHand, GripInfo);
	if(bIsPrimaryGrip)
	{		
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(AllGrips[0].GrippedObject))
		{
			MyGun->OnMagReleasePressedFromPrimary();
		}
		else if(ATVRMagazine* MyMag = Cast<ATVRMagazine>(AllGrips[0].GrippedObject))
		{
			MyMag->OnMagReleasePressed();
		}
	}
	else if(bIsSecondaryGrip)
	{
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(GripInfo.GrippedObject))
		{
			if(!MyGun->OnSecondaryMagReleasePressed())
			{
				MyGun->ToggleLight(UsingHand);
			}
		}
	}
	else
	{
		USphereComponent* GrabSphere = GetGrabSphere(UsingHand);
		check(GrabSphere);
		if(UTVRHoverInputVolume* TestInputVol = GetOverlappingHoverInputComp(GrabSphere))
		{
			if(!TestInputVol->OnMagReleasePressed(UsingHand))
			{
				TestInputVol->OnLightPressed(UsingHand);
			}
		}
		else
		{
			// do something when hands are fully empty?
		}
	}
}

void ATVRCharacter::OnActionB_Pressed(UGripMotionControllerComponent* UsingHand)
{
	check(UsingHand);
	TArray<FBPActorGripInformation> AllGrips;
	FBPActorGripInformation GripInfo;
	
	UsingHand->GetAllGrips(AllGrips);
	
	const bool bIsPrimaryGrip = AllGrips.Num() > 0;
	const bool bIsSecondaryGrip = GetOtherControllerHand(UsingHand)->GetIsSecondaryAttachment(UsingHand, GripInfo);
	if(bIsPrimaryGrip)
	{		
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(AllGrips[0].GrippedObject))
		{
			if(MyGun->HasBoltReleaseOnPrimaryGrip())
			{
				MyGun->OnBoltReleasePressed();
			}
		}
	}
	else if(bIsSecondaryGrip)
	{
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(GripInfo.GrippedObject))
		{
			MyGun->ToggleLaser(UsingHand);
		}
	}
	else if(USphereComponent* GrabSphere = GetGrabSphere(UsingHand))
	{
		if(UTVRHoverInputVolume* TestInputVol = GetOverlappingHoverInputComp(GrabSphere))
		{
			if(!TestInputVol->OnBoltReleasePressed(UsingHand))
			{
				TestInputVol->OnLaserPressed(UsingHand);
			}
		}
		else
		{
			TogglePauseMenu(UsingHand);
		}
	}
}

void ATVRCharacter::OnActionA_Released(UGripMotionControllerComponent* UsingHand)
{
	check(UsingHand);
	TArray<FBPActorGripInformation> AllGrips;
	FBPActorGripInformation GripInfo;
	
	UsingHand->GetAllGrips(AllGrips);
	
	const bool bIsPrimaryGrip = AllGrips.Num() > 0;
	const bool bIsSecondaryGrip = GetOtherControllerHand(UsingHand)->GetIsSecondaryAttachment(UsingHand, GripInfo);
	if(bIsPrimaryGrip)
	{		
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(AllGrips[0].GrippedObject))
		{
			MyGun->OnMagReleaseReleasedFromPrimary();
		}
		else if(ATVRMagazine* MyMag = Cast<ATVRMagazine>(AllGrips[0].GrippedObject))
		{
			MyMag->OnMagReleaseReleased();
		}
	}
	else if(bIsSecondaryGrip)
	{
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(GripInfo.GrippedObject))
		{
			MyGun->OnSecondaryMagReleaseReleased();
		}
	}
	else
	{
		USphereComponent* GrabSphere = GetGrabSphere(UsingHand);
		check(GrabSphere);
		if(UTVRHoverInputVolume* TestInputVol = GetOverlappingHoverInputComp(GrabSphere))
		{
			if(!TestInputVol->OnMagReleaseReleased(UsingHand))
			{
				// TestInputVol->OnLightPressed(UsingHand);
			}
		}
		else
		{
			// do something when hands are fully empty?
		}
	}
}

void ATVRCharacter::OnActionB_Released(UGripMotionControllerComponent* UsingHand)
{
	check(UsingHand);
	TArray<FBPActorGripInformation> AllGrips;
	FBPActorGripInformation GripInfo;
	
	UsingHand->GetAllGrips(AllGrips);
	
	const bool bHasPrimaryGrip = AllGrips.Num() > 0;
	const bool bHasSecondaryGrip = GetOtherControllerHand(UsingHand)->GetIsSecondaryAttachment(UsingHand, GripInfo);
	if(bHasPrimaryGrip)
	{		
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(AllGrips[0].GrippedObject))
		{
			if(MyGun->HasBoltReleaseOnPrimaryGrip())
			{
				MyGun->OnBoltReleaseReleased();
			}
		}
	}
	else if(bHasSecondaryGrip)
	{
		if(ATVRGunBase* MyGun = Cast<ATVRGunBase>(GripInfo.GrippedObject))
		{
			MyGun->OnSecondaryBoltReleaseReleased();
		}
	}
	else
	{
		USphereComponent* GrabSphere = GetGrabSphere(UsingHand);
		check(GrabSphere);
		if(UTVRHoverInputVolume* TestInputVol = GetOverlappingHoverInputComp(GrabSphere))
		{
			if(!TestInputVol->OnBoltReleaseReleased(UsingHand))
			{
				// TestInputVol->OnLightPressed(UsingHand);
			}
		}
		else
		{
			// do something when hands are fully empty?
		}
	}
	ReleaseTogglePauseMenu(UsingHand);
}


void ATVRCharacter::OnCycleFireMode_Left()
{    
	OnCycleFireMode(LeftMotionController);
}

void ATVRCharacter::OnCycleFireMode_Right()
{
	OnCycleFireMode(RightMotionController);
}

void ATVRCharacter::OnCycleFireMode(UGripMotionControllerComponent* UsingHand)
{
	TArray<FBPActorGripInformation> AllGrips;
	if(UsingHand != nullptr)
	{
		UsingHand->GetAllGrips(AllGrips);
		if(AllGrips.Num() > 0)
		{
			ATVRGunBase* MyGun = Cast<ATVRGunBase>(AllGrips[0].GrippedObject);
			if(MyGun != nullptr)
			{
				MyGun->OnCycleFiringMode();
			}
		}
	}
}

void ATVRCharacter::SpawnGraspingHands()
{
    DestroyGraspingHands();

	const bool bUseCurls = true;
	
    if(RightGraspingHandClass != nullptr)
    {
    	// const FTransform Correction = FTransform(
    	// 	FRotator(0.f, 30.f, 0.f),
		// 	FVector(0.f, -3.f, 0.f)
    	// );
    	const FTransform Correction = FTransform::Identity;
        const FTransform RHTransform = Correction * HandMeshRight->GetComponentTransform();
        const FTransform LHTransform = Correction * HandMeshLeft->GetComponentTransform();
        
        RightGraspingHand = GetWorld()->SpawnActorDeferred<ATVRGraspingHand>(RightGraspingHandClass, RHTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        if(RightGraspingHand)
        {
            RightGraspingHand->PhysicsRoot = GrabSphereRight;
            RightGraspingHand->OwningController = RightMotionController;
            RightGraspingHand->OtherController = LeftMotionController;
            RightGraspingHand->bUseCurls = bUseCurls;
            RightGraspingHand->FinishSpawning(RHTransform);
            GrabSphereRight->SetHiddenInGame(true);
            HandMeshRight->SetHiddenInGame(true);
            // todo procedural mesh hidden in game
            RightGraspingHand->AttachToComponent(RightMotionController, FAttachmentTransformRules::KeepWorldTransform);
        }

        LeftGraspingHand = GetWorld()->SpawnActorDeferred<ATVRGraspingHand>(RightGraspingHandClass, LHTransform,  this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        if(LeftGraspingHand)
        {
            LeftGraspingHand->PhysicsRoot = GrabSphereLeft;
            LeftGraspingHand->OwningController = LeftMotionController;
            LeftGraspingHand->OtherController = RightMotionController;
            LeftGraspingHand->bUseCurls = bUseCurls;
            LeftGraspingHand->FinishSpawning(LHTransform);            
            GrabSphereLeft->SetHiddenInGame(true);
            HandMeshLeft->SetHiddenInGame(true);
            // todo procedural mesh hidden in game
            LeftGraspingHand->AttachToComponent(LeftMotionController, FAttachmentTransformRules::KeepWorldTransform);
        }
    }
}

void ATVRCharacter::DestroyGraspingHands()
{
    if(RightGraspingHand != nullptr)
    {
        RightGraspingHand->Destroy();
        RightGraspingHand = nullptr;
        HandMeshRight->SetHiddenInGame(false);        
    }
    if(LeftGraspingHand != nullptr)
    {
        LeftGraspingHand->Destroy();
        LeftGraspingHand = nullptr;
        HandMeshLeft->SetHiddenInGame(false);        
    }
    if(GrabSphereRight)
    {
        RightMotionController->SetCustomPivotComponent(GrabSphereRight);
    }
    if(GrabSphereLeft)
    {
        LeftMotionController->SetCustomPivotComponent(GrabSphereLeft);
    }
}
