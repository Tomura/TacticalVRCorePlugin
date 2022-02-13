// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_Light.h"
#include "Components/SpotLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TVRHoverInputVolume.h"

AWPNA_Light::AWPNA_Light(const FObjectInitializer& OI) : Super(OI)
{
	HoverInputComponent = CreateDefaultSubobject<UTVRHoverInputVolume>(FName("HoverInputComponent"));
	HoverInputComponent->SetupAttachment(GetStaticMeshComponent());
	HoverInputComponent->SetSphereRadius(5.f);
	
	LaserToggleSound = CreateDefaultSubobject<UAudioComponent>(FName("LaserToggleSound"));
	LaserToggleSound->SetupAttachment(GetStaticMeshComponent());
	LaserToggleSound->bAutoActivate = false;

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(FName("SpotLight"));
	SpotLight->SetupAttachment(GetStaticMeshComponent());
	SpotLight->SetVisibility(false);
	
	PrimaryActorTick.bCanEverTick = false;
}

void AWPNA_Light::BeginPlay()
{
	Super::BeginPlay();

	HoverInputComponent->EventOnUsed.AddDynamic(this, &AWPNA_Light::ToggleLight);
	HoverInputComponent->EventOnLightPressed.AddDynamic(this, &AWPNA_Light::ToggleLight);
}

void AWPNA_Light::ToggleLight(UGripMotionControllerComponent* UsingHand)
{
	if(LaserToggleSound->Sound)
	{
		LaserToggleSound->Play();
	}
	if(IsLightOn())
	{
		bIsLightOn = false;
		SpotLight->SetVisibility(false);
		SetActorTickEnabled(false);
	}
	else
	{
		bIsLightOn = true;
		SpotLight->SetVisibility(true);
		SetActorTickEnabled(true);
	}
}
