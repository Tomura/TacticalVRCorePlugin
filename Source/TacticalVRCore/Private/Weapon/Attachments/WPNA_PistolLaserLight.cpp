// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_PistolLaserLight.h"

#include "TacticalCollisionProfiles.h"
#include "Components/TVRHoverInputVolume.h"
#include "Components/AudioComponent.h"
#include "Weapon/TVRGunBase.h"

#define MAX_TRACE_DISTANCE 5000.f
#define MAX_THICKNESS_TPP 0.01f

// Sets default values
AWPNA_PistolLaserLight::AWPNA_PistolLaserLight(const FObjectInitializer& OI) : Super(OI)
{
	LaserBeam = CreateDefaultSubobject<UStaticMeshComponent>(FName("LaserBeamMesh"));
	LaserBeam->SetupAttachment(GetStaticMeshComponent());
	LaserBeam->SetCollisionProfileName(COLLISION_NO_COLLISION);
	LaserBeam->SetVisibility(false);

	LaserImpactMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LaserBeamImpactMesh"));
	LaserImpactMesh->SetupAttachment(LaserBeam);
	LaserImpactMesh->SetCollisionProfileName(COLLISION_NO_COLLISION);
	LaserImpactMesh->SetVisibility(false);
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bIsLaserOn = false;	
	Spread = 0.000015f;
	BaseThickness = 0.002f;
}

// Called when the game starts or when spawned
void AWPNA_PistolLaserLight::BeginPlay()
{
	Super::BeginPlay();
	
	// HoverInputComponent->EventOnUsed.AddDynamic(this, &AWPNA_Laser::ToggleLaser);
	GetHoverInputComponent()->EventOnLaserPressed.AddDynamic(this, &AWPNA_PistolLaserLight::ToggleLaser);
}

// Called every frame
void AWPNA_PistolLaserLight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
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

void AWPNA_PistolLaserLight::ToggleLaser(UGripMotionControllerComponent* UsingHand)
{
	if(GetToggleSound()->Sound)
	{
		GetToggleSound()->Play();
	}
	if(IsLaserOn())
	{
		bIsLaserOn = false;
		LaserBeam->SetVisibility(false);
		LaserImpactMesh->SetVisibility(false);
		SetActorTickEnabled(false);
	}
	else
	{
		bIsLaserOn = true;
		LaserBeam->SetVisibility(true);
		LaserImpactMesh->SetVisibility(true);
		SetActorTickEnabled(true);
	}
}
