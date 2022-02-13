// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_PistolLight.h"
#include "Weapon/Attachments/WPNA_PistolLight.h"


// Sets default values for this component's properties
UTVRAttachPoint_PistolLight::UTVRAttachPoint_PistolLight(const FObjectInitializer& OI): Super(OI)
{
}

// Called when the game starts
void UTVRAttachPoint_PistolLight::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

#if WITH_EDITOR
void UTVRAttachPoint_PistolLight::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UTVRAttachPoint_PistolLight, CurrentLightClass))
	{
		SetChildActorClass(CurrentLightClass);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UTVRAttachPoint_PistolLight::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UTVRAttachPoint_PistolLight, CurrentLightClass))
	{
		SetChildActorClass(CurrentLightClass);
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_PistolLight::GetCurrentAttachmentClass() const
{
	return CurrentLightClass;
}

void UTVRAttachPoint_PistolLight::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& AllowedAttachments) const
{
	for(TSubclassOf<AWPNA_PistolLight> TestClass : AllowedLights)
	{
		AllowedAttachments.Add(TestClass);
	}
}

bool UTVRAttachPoint_PistolLight::ToggleLaser()
{
	AWPNA_PistolLight* PistolLight = GetCurrentAttachment() ? Cast<AWPNA_PistolLight>(GetCurrentAttachment()) : nullptr;
	if(PistolLight)
	{
		PistolLight->ToggleLaser(nullptr);
		return true;
	}
	return false;
}

bool UTVRAttachPoint_PistolLight::ToggleLight()
{
	AWPNA_PistolLight* PistolLight = GetCurrentAttachment() ? Cast<AWPNA_PistolLight>(GetCurrentAttachment()) : nullptr;
	if(PistolLight)
	{
		PistolLight->ToggleLight(nullptr);
		return true;
	}
	return false;
}
