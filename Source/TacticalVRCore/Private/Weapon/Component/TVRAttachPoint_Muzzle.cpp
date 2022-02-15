// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Muzzle.h"
#include "Weapon/Attachments/WPNA_Muzzle.h"

UTVRAttachPoint_Muzzle::UTVRAttachPoint_Muzzle(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Muzzle::GetCurrentAttachmentClass() const
{
	return CurrentAttachmentClass;
}

void UTVRAttachPoint_Muzzle::GetAllowedAttachments(
	TArray<TSubclassOf<ATVRWeaponAttachment>>& OutAllowedAttachments) const
{
	for(TSubclassOf<AWPNA_Muzzle> TestClass : AllowedMuzzles)
	{
		OutAllowedAttachments.Add(TestClass);
	}
}
