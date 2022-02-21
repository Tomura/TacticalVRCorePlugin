// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Muzzle.h"
#include "Weapon/Attachments/WPNA_Muzzle.h"

UTVRAttachPoint_Muzzle::UTVRAttachPoint_Muzzle(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

bool UTVRAttachPoint_Muzzle::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	if(NewClass == nullptr)
	{
		CurrentAttachmentClass = nullptr;
		OnConstruction();
		return true;
	}
	
	if(NewClass->IsChildOf(AWPNA_Muzzle::StaticClass()))
	{
		const TSubclassOf<AWPNA_Muzzle> TestClass = *NewClass;
		if(AllowedMuzzles.Find(TestClass) != INDEX_NONE)
		{
			CurrentAttachmentClass = TestClass;
			OnConstruction();
			return true;
		}
	}
	return false;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Muzzle::GetCurrentAttachmentClass_Internal() const
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
