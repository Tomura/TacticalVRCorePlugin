// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Sight.h"

#include "Weapon/TVRGunBase.h"
#include "Weapon/Attachments/WPNA_Sight.h"


// Sets default values for this component's properties
UTVRAttachPoint_Sight::UTVRAttachPoint_Sight(const FObjectInitializer& OI): Super(OI)
{
	CurrentSightClass = nullptr;
}

bool UTVRAttachPoint_Sight::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	if(NewClass == nullptr)
	{
		CurrentSightClass = nullptr;
		OnConstruction();
		return true;
	}
	
	if(NewClass->IsChildOf(AWPNA_Sight::StaticClass()))
	{
		const TSubclassOf<AWPNA_Sight> TestClass = *NewClass;
		if(AllowedSights.Find(TestClass) != INDEX_NONE)
		{
			CurrentSightClass = TestClass;
			OnConstruction();
			return true;
		}
	}
	return false;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Sight::GetCurrentAttachmentClass_Internal() const
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


