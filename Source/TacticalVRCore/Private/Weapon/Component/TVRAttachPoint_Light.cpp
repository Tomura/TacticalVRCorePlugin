// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Light.h"
#include "Weapon/Attachments/WPNA_Light.h"


// Sets default values for this component's properties
UTVRAttachPoint_Light::UTVRAttachPoint_Light(const FObjectInitializer& OI): Super(OI)
{
	CurrentLightClass = nullptr;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Light::GetCurrentAttachmentClass() const
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


