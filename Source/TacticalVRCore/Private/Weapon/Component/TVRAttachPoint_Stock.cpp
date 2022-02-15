// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Weapon/Component/TVRAttachPoint_Stock.h"
#include "Weapon/Attachments/WPNA_Stock.h"

UTVRAttachPoint_Stock::UTVRAttachPoint_Stock(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}


TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Stock::GetCurrentAttachmentClass() const
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