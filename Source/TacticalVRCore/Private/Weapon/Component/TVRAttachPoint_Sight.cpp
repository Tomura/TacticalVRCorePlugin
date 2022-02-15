// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Sight.h"

#include "Weapon/TVRGunBase.h"
#include "Weapon/Attachments/WPNA_Sight.h"


// Sets default values for this component's properties
UTVRAttachPoint_Sight::UTVRAttachPoint_Sight(const FObjectInitializer& OI): Super(OI)
{
	CurrentSightClass = nullptr;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Sight::GetCurrentAttachmentClass() const
{
	return CurrentSightClass;
}

void UTVRAttachPoint_Sight::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& AllowedAttachments) const
{
	for(TSubclassOf<AWPNA_Sight> TestSightClass : AllowedSights)
	{
		AllowedAttachments.Add(TestSightClass);
	}
}


