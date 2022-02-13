// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_Sight.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Component/TVRAttachmentPoint.h"

AWPNA_Sight::AWPNA_Sight(const FObjectInitializer& OI) : Super(OI)
{
	bFoldIronSights = true;
	bHideRearSight = false;
}

void AWPNA_Sight::Destroyed()
{
	ATVRGunBase* Gun = GetGunOwner();
	if(Gun)
	{
		Gun->FoldSights(false);
		Gun->HideRearSight(false);
	}
	Super::Destroyed();
}

void AWPNA_Sight::AttachToWeapon(UTVRAttachmentPoint* AttachPoint)
{
	Super::AttachToWeapon(AttachPoint);
	ATVRGunBase* Gun = GetGunOwner();
	if(Gun)
	{
		Gun->FoldSights(bFoldIronSights);
		Gun->HideRearSight(bHideRearSight);
	}
}
