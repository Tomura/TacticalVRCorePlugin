// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Weapon/Component/TVRAttachPoint_Stock.h"
#include "Weapon/Attachments/WPNA_Stock.h"

UTVRAttachPoint_Stock::UTVRAttachPoint_Stock(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

bool UTVRAttachPoint_Stock::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	if(NewClass == nullptr)
	{
		CurrentAttachmentClass = nullptr;
		OnConstruction();
		return true;
	}
	
	if(NewClass->IsChildOf(AWPNA_Stock::StaticClass()))
	{
		const TSubclassOf<AWPNA_Stock> TestClass = *NewClass;
		if(AllowedAttachments.Find(TestClass) != INDEX_NONE)
		{
			CurrentAttachmentClass = TestClass;
			OnConstruction();
			return true;
		}
	}
	return false;
}


TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Stock::GetCurrentAttachmentClass_Internal() const
{
	return CurrentAttachmentClass;
}

void UTVRAttachPoint_Stock::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& OutAllowedAttachments) const
{
	for(const auto TestAttachmentClass : AllowedAttachments)
	{
		OutAllowedAttachments.Add(TestAttachmentClass);
	}
}