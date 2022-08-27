// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Weapon/Component/TVRAttachPoint_Barrel.h"

#include "Weapon/TVRGunBase.h"
#include "Weapon/Attachments/WPNA_Barrel.h"

UTVRAttachPoint_Barrel::UTVRAttachPoint_Barrel(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

bool UTVRAttachPoint_Barrel::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	if(NewClass == nullptr)
	{
		CurrentAttachmentClass = nullptr;
		OnConstruction();
		return true;
	}
	
	if(NewClass->IsChildOf(AWPNA_Barrel::StaticClass()))
	{
		const TSubclassOf<AWPNA_Barrel> TestClass = *NewClass;
		if(AllowedAttachments.Find(TestClass) != INDEX_NONE)
		{
			CurrentAttachmentClass = TestClass;
			OnConstruction();			
			return true;
		}
	}
	return false;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Barrel::GetCurrentAttachmentClass_Internal() const
{
	return CurrentAttachmentClass;
}

void UTVRAttachPoint_Barrel::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& OutAllowedAttachments) const
{
	for(TSubclassOf<AWPNA_Barrel> TestSightClass : AllowedAttachments)
	{
		OutAllowedAttachments.Add(TestSightClass);
	}
}

void UTVRAttachPoint_Barrel::OnConstruction()
{
	Super::OnConstruction();
	
	if(GetGunOwner())
	{
		GetGunOwner()->OnBarrelChanged(CurrentAttachmentClass);
	}
}
