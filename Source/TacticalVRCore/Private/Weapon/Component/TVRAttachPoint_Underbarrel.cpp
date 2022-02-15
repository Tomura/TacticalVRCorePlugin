// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Underbarrel.h"
#include "Weapon/Attachments/WPNA_ForeGrip.h"

UTVRAttachPoint_Underbarrel::UTVRAttachPoint_Underbarrel(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Underbarrel::GetCurrentAttachmentClass() const
{
	return CurrentAttachmentClass;
}

void UTVRAttachPoint_Underbarrel::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& OutAllowedAttachments) const
{
	for(TSubclassOf<AWPNA_ForeGrip> TestSightClass : AllowedAttachments)
	{
		OutAllowedAttachments.Add(TestSightClass);
	}
}
