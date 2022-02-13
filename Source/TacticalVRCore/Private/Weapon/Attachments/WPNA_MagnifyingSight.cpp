// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_MagnifyingSight.h"


#include "AnimationCompression.h"
#include "GripMotionControllerComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Player/TVRCharacter.h"
#include "Weapon/TVRGunBase.h"

AWPNA_MagnifyingSight::AWPNA_MagnifyingSight(const FObjectInitializer& OI): Super(OI)
{
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("SceneCaptureComponent"));
	SceneCaptureComponent->SetupAttachment(GetStaticMeshComponent());
	SceneCaptureComponent->SetVisibility(false);
	SceneCaptureComponent->SetRelativeScale3D(FVector(0.1f));

	PrimaryActorTick.bCanEverTick = true;
	
	ReticleMaterial = nullptr;
	ReticleMaterialTemplate = nullptr;
	ReticleMaterialPassive = nullptr;
	bUseSimpleApproach=false;
	
	bFirstFocalPointReticle = false;
	bIsVariableOptic = false;
	ApertureRadius = 2.f;
	ZoomFactor = 1.f;
	BaseFactor1x = 2.f;
}

void AWPNA_MagnifyingSight::BeginPlay()
{
	Super::BeginPlay();
	// these are mandatory, so we want to throw during development
	check(ReticleMaterialPassive);
	check(ReticleMaterialTemplate);
	ReticleMaterial = UMaterialInstanceDynamic::Create(ReticleMaterialTemplate, this);
	SetReticleMaterial(ReticleMaterialPassive);
}

void AWPNA_MagnifyingSight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(SceneCaptureComponent->IsVisible())
	{
		ATVRGunBase* Gun = GetGunOwner();
		if(Gun && Gun->IsOwnerLocalPlayerController())
		{
			ATVRCharacter* CharacterOwner = Gun->GetVRCharacterOwner();
			if(CharacterOwner)
			{
				const FVector ApertureLoc = SceneCaptureComponent->GetComponentLocation();
				const FVector RightEye = CharacterOwner->VRReplicatedCamera->GetComponentLocation() + CharacterOwner->VRReplicatedCamera->GetRightVector() * 3.2;
				const FVector LeftEye = CharacterOwner->VRReplicatedCamera->GetComponentLocation() - CharacterOwner->VRReplicatedCamera->GetRightVector() * 3.2;
				const FVector CameraVec = FMath::ClosestPointOnSegment(ApertureLoc, RightEye, LeftEye);				
				const FVector Cam2SightVec = ApertureLoc - CameraVec;

				float HalfAngleRad;
				if(!bUseSimpleApproach)
				{
					const FVector SightRightVec = SceneCaptureComponent->GetRightVector();
					const FVector Right = Cam2SightVec + ApertureRadius * SightRightVec;
					const FVector Left = Cam2SightVec - ApertureRadius * SightRightVec;
					HalfAngleRad = 0.5f * FMath::Acos(Right.GetSafeNormal() | Left.GetSafeNormal());
				}
				else
				{
					HalfAngleRad = FMath::Atan(ApertureRadius/Cam2SightVec.Size());
				}
				SceneCaptureComponent->FOVAngle = 2.f * FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(HalfAngleRad * BaseFactor1x) / ZoomFactor));
				
				if(bFirstFocalPointReticle && ReticleMaterial)
				{
					ReticleMaterial->SetScalarParameterValue(FName("ZoomLevel"), ZoomFactor);
				}
			}
		}
	}
}

void AWPNA_MagnifyingSight::OnOwnerGripped_Implementation(UGripMotionControllerComponent* GrippingHand,
                                           const FBPActorGripInformation& GripInfo)
{
	if(GrippingHand && GrippingHand->IsLocallyControlled())
	{
		SceneCaptureComponent->SetVisibility(true, false);
		SetActorTickEnabled(true);
		SetReticleMaterial(ReticleMaterial);
	}
}

void AWPNA_MagnifyingSight::OnOwnerDropped_Implementation(UGripMotionControllerComponent* GrippingHand,
	const FBPActorGripInformation& GripInfo, bool bSocketed)
{
	if(GrippingHand && GrippingHand->IsLocallyControlled())
	{
		SceneCaptureComponent->SetVisibility(false, false);
		SetActorTickEnabled(false);
		SetReticleMaterial(ReticleMaterialPassive);
	}
}

float AWPNA_MagnifyingSight::GetZoomLevelFromCurve(float Time) const
{
	return ZoomDialCurve.GetRichCurveConst()->Eval(Time);
}
