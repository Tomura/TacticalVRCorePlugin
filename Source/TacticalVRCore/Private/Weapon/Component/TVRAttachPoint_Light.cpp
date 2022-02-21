// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Light.h"
#include "Weapon/Attachments/WPNA_Light.h"


// Sets default values for this component's properties
UTVRAttachPoint_Light::UTVRAttachPoint_Light(const FObjectInitializer& OI): Super(OI)
{
	CurrentLightClass = nullptr;
}

bool UTVRAttachPoint_Light::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	if(NewClass == nullptr)
	{
		CurrentLightClass = nullptr;
		OnConstruction();
		return true;
	}
	
	if(NewClass->IsChildOf(AWPNA_Light::StaticClass()))
	{
		const TSubclassOf<AWPNA_Light> TestClass = *NewClass;
		if(AllowedLights.Find(TestClass) != INDEX_NONE)
		{
			CurrentLightClass = TestClass;
			OnConstruction();
			return true;
		}
	}
	return false;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Light::GetCurrentAttachmentClass_Internal() const
{
	return CurrentLightClass;
}

void UTVRAttachPoint_Light::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& AllowedAttachments) const
{
	for(TSubclassOf<AWPNA_Light> TestClass : AllowedLights)
	{
		AllowedAttachments.Add(TestClass);
	}
}

bool UTVRAttachPoint_Light::ToggleLight()
{
	AWPNA_Light* WeaponLight = GetCurrentAttachment() ? Cast<AWPNA_Light>(GetCurrentAttachment()) : nullptr;
	if(WeaponLight)
	{
		WeaponLight->ToggleLight(nullptr);
		return true;
	}
	return false;
}


