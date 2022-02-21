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
	
	LightToggleSound = CreateDefaultSubobject<UAudioComponent>(FName("LightToggleSound"));
	LightToggleSound->SetupAttachment(GetStaticMeshComponent());
	LightToggleSound->bAutoActivate = false;

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(FName("SpotLight"));
	SpotLight->SetupAttachment(GetStaticMeshComponent());
	SpotLight->SetVisibility(false);
	
	PrimaryActorTick.bCanEverTick = false;
	
	LightMaterialInstance = nullptr;
	LightOnMaterialParam = FName(TEXT("LightOn"));
	LightMaterialSlot = 0;
}

void AWPNA_Light::BeginPlay()
{
	Super::BeginPlay();

	HoverInputComponent->EventOnUsed.AddDynamic(this, &AWPNA_Light::ToggleLight);
	HoverInputComponent->EventOnLightPressed.AddDynamic(this, &AWPNA_Light::ToggleLight);

	if(const auto TempMat = GetStaticMeshComponent()->GetMaterial(LightMaterialSlot))
	{
		LightMaterialInstance = GetStaticMeshComponent()->CreateDynamicMaterialInstance(LightMaterialSlot);
		LightMaterialInstance->SetScalarParameterValue(LightOnMaterialParam, IsLightOn() ? 1.f : 0.f);
	}
}

void AWPNA_Light::ToggleLight(UGripMotionControllerComponent* UsingHand)
{
	if(LightToggleSound->Sound)
	{
		LightToggleSound->Play();
	}
	if(IsLightOn())
	{
		bIsLightOn = false;
		SpotLight->SetVisibility(false);
		SetActorTickEnabled(false);		
		if(LightMaterialInstance)
		{
			LightMaterialInstance->SetScalarParameterValue(LightOnMaterialParam, 0.f);
		}
	}
	else
	{
		bIsLightOn = true;
		SpotLight->SetVisibility(true);
		SetActorTickEnabled(true);		
		if(LightMaterialInstance)
		{
			LightMaterialInstance->SetScalarParameterValue(LightOnMaterialParam, 1.f);
		}
	}
}
