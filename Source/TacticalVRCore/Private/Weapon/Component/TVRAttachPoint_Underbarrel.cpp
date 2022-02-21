// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Underbarrel.h"
#include "Weapon/Attachments/WPNA_ForeGrip.h"

UTVRAttachPoint_Underbarrel::UTVRAttachPoint_Underbarrel(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

bool UTVRAttachPoint_Underbarrel::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	if(NewClass == nullptr)
	{
		CurrentAttachmentClass = nullptr;
		OnConstruction();
		return true;
	}
	
	if(NewClass->IsChildOf(AWPNA_ForeGrip::StaticClass()))
	{
		const TSubclassOf<AWPNA_ForeGrip> TestClass = *NewClass;
		if(AllowedAttachments.Find(TestClass) != INDEX_NONE)
		{
			CurrentAttachmentClass = TestClass;
			OnConstruction();
			return true;
		}
	}
	return false;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Underbarrel::GetCurrentAttachmentClass_Internal() const
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
