﻿// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Laser.h"
#include "Weapon/Attachments/WPNA_Laser.h"


// Sets default values for this component's properties
UTVRAttachPoint_Laser::UTVRAttachPoint_Laser(const FObjectInitializer& OI): Super(OI)
{
	CurrentLaserClass = nullptr;
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Laser::GetCurrentAttachmentClass() const
{
	return CurrentLaserClass;
}

void UTVRAttachPoint_Laser::GetAllowedAttachments(TArray<TSubclassOf<ATVRWeaponAttachment>>& AllowedAttachments) const
{
	for(TSubclassOf<AWPNA_Laser> TestClass : AllowedLasers)
	{
		AllowedAttachments.Add(TestClass);
	}
}

bool UTVRAttachPoint_Laser::ToggleLaser()
{
	AWPNA_Laser* Laser = GetCurrentAttachment() ? Cast<AWPNA_Laser>(GetCurrentAttachment()) : nullptr;
	if(Laser)
	{
		Laser->ToggleLaser(nullptr);
		return true;
	}
	return false;
}

bool UTVRAttachPoint_Laser::SetCurrentLaser(TSubclassOf<AWPNA_Laser> NewAttachment, bool bForce)
{
	if(bForce || AllowedLasers.Find(NewAttachment) != INDEX_NONE)
	{
		CurrentLaserClass = NewAttachment;
		SetChildActorClass(CurrentLaserClass);
		return true;
	}
	return false;
}

