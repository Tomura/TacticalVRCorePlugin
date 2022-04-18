// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_Laser.h"

#include "TacticalCollisionProfiles.h"
#include "Components/TVRHoverInputVolume.h"
#include "Components/AudioComponent.h"
#include "Weapon/TVRGunBase.h"

#define MAX_TRACE_DISTANCE 5000.f
#define MAX_THICKNESS_TPP 0.01f

AWPNA_Laser::AWPNA_Laser(const FObjectInitializer& OI) : Super(OI)
{
	HoverInputComponent = CreateDefaultSubobject<UTVRHoverInputVolume>(FName("HoverInputComponent"));
	HoverInputComponent->SetupAttachment(GetStaticMeshComponent());
	HoverInputComponent->SetSphereRadius(5.f);
	
	LaserBeam = CreateDefaultSubobject<UStaticMeshComponent>(FName("LaserBeamMesh"));
	LaserBeam->SetupAttachment(GetStaticMeshComponent());
	LaserBeam->SetCollisionProfileName(COLLISION_NO_COLLISION);
	LaserBeam->SetVisibility(false);

	LaserImpactMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LaserBeamImpactMesh"));
	LaserImpactMesh->SetupAttachment(LaserBeam);
	LaserImpactMesh->SetCollisionProfileName(COLLISION_NO_COLLISION);
	LaserImpactMesh->SetVisibility(false);
	

	LaserToggleSound = CreateDefaultSubobject<UAudioComponent>(FName("LaserToggleSound"));
	LaserToggleSound->SetupAttachment(GetStaticMeshComponent());
	LaserToggleSound->bAutoActivate = false;

	PrimaryActorTick.bCanEverTick = true;

	Spread = 0.000015f;
	BaseThickness = 0.002f;
	bIsLaserOn = false;

	LaserMaterialInstance = nullptr;
	LaserOnMaterialParam = FName(TEXT("LaserOn"));
	LaserMaterialSlot = 0;
}

void AWPNA_Laser::BeginPlay()
{
	Super::BeginPlay();

	HoverInputComponent->EventOnUsed.AddDynamic(this, &AWPNA_Laser::ToggleLaser);
	HoverInputComponent->EventOnLaserPressed.AddDynamic(this, &AWPNA_Laser::ToggleLaser);

	if(const auto TempMat = GetStaticMeshComponent()->GetMaterial(LaserMaterialSlot))
	{
		LaserMaterialInstance = GetStaticMeshComponent()->CreateDynamicMaterialInstance(LaserMaterialSlot);
		LaserMaterialInstance->SetScalarParameterValue(LaserOnMaterialParam, IsLaserOn() ? 1.f : 0.f);
	}
}

void AWPNA_Laser::Tick(float DeltaSeconds)
{
	if(IsLaserOn())
	{
		FHitResult TraceResult;
		const FVector TraceStart = LaserBeam->GetComponentLocation();
		const FVector TraceEnd = TraceStart + LaserBeam->GetForwardVector() * MAX_TRACE_DISTANCE;
		FCollisionQueryParams QueryParams = FCollisionQueryParams(FName("TraceLaserBeam"), false, this);
		const ATVRGunBase* GunOwner = GetGunOwner();
		if(GunOwner)
		{
			QueryParams.AddIgnoredActor(GunOwner);
		}
		bool bSuccessfulHit = false;
		if(GetWorld()->LineTraceSingleByChannel(TraceResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			if(TraceResult.bBlockingHit)
			{
				bSuccessfulHit = true;
				FVector NewScale = LaserBeam->GetRelativeScale3D();
				NewScale.X = TraceResult.Distance;
				LaserBeam->SetRelativeScale3D(NewScale);
				float LightRadius = BaseThickness + TraceResult.Distance * Spread;

				if(!LaserImpactMesh->IsVisible())
				{
					LaserImpactMesh->SetVisibility(true);
				}
				if(!GunOwner || !GunOwner->IsOwnerLocalPlayerController())
				{
					LightRadius = FMath::Min(LightRadius, MAX_THICKNESS_TPP);
				}
				LaserImpactMesh->SetWorldScale3D(FVector(LightRadius));
				LaserImpactMesh->SetWorldLocation(TraceResult.ImpactPoint);
			}
			PostLaserTrace(TraceResult);
		}
		if(!bSuccessfulHit)
		{
			FVector NewScale = LaserBeam->GetRelativeScale3D();
			NewScale.X = MAX_TRACE_DISTANCE;
			LaserBeam->SetRelativeScale3D(NewScale);
			if(LaserImpactMesh->IsVisible())
			{
				LaserImpactMesh->SetVisibility(false);
			}
		}
	}
}

void AWPNA_Laser::ToggleLaser(UGripMotionControllerComponent* UsingHand)
{
	if(LaserToggleSound->Sound)
	{
		LaserToggleSound->Play();
	}
	if(IsLaserOn())
	{
		bIsLaserOn = false;
		LaserBeam->SetVisibility(false);
		LaserImpactMesh->SetVisibility(false);
		SetActorTickEnabled(false);
		if(LaserMaterialInstance)
		{
			LaserMaterialInstance->SetScalarParameterValue(LaserOnMaterialParam, 0.f);
		}
		OnToggleLaser(false, UsingHand);
		if(EventOnToggleLaser.IsBound())
		{
			EventOnToggleLaser.Broadcast(false, UsingHand);
		}
	}
	else
	{
		bIsLaserOn = true;
		LaserBeam->SetVisibility(true);
		LaserImpactMesh->SetVisibility(true);
		SetActorTickEnabled(true);		
		if(LaserMaterialInstance)
		{
			LaserMaterialInstance->SetScalarParameterValue(LaserOnMaterialParam, 1.f);
		}		
		OnToggleLaser(true, UsingHand);
		if(EventOnToggleLaser.IsBound())
		{
			EventOnToggleLaser.Broadcast(true, UsingHand);
		}
	}
}
