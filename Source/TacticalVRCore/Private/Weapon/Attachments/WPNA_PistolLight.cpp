// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_PistolLight.h"
#include "Components/AudioComponent.h"
#include "Components/TVRHoverInputVolume.h"
#include "Components/SpotLightComponent.h"
#include "Weapon/TVRGunBase.h"


AWPNA_PistolLight::AWPNA_PistolLight(const FObjectInitializer& OI) : Super(OI)
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

void AWPNA_PistolLight::BeginPlay()
{
	Super::BeginPlay();

	// HoverInputComponent->EventOnUsed.AddDynamic(this, &AWPNA_PistolLight::ToggleLight);
	HoverInputComponent->EventOnLightPressed.AddDynamic(this, &AWPNA_PistolLight::ToggleLight);
}

void AWPNA_PistolLight::ToggleLight(UGripMotionControllerComponent* UsingHand)
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

