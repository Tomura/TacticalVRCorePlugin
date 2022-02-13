// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_Sight.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_Sight : public ATVRWeaponAttachment
{
	GENERATED_BODY()
public:
	AWPNA_Sight(const FObjectInitializer& OI);
	virtual void Destroyed() override;
	virtual void AttachToWeapon(UTVRAttachmentPoint* AttachPoint) override;

	UPROPERTY(Category = "Sight", EditDefaultsOnly)
	bool bFoldIronSights;
	
	UPROPERTY(Category = "Sight", EditDefaultsOnly)
	bool bHideRearSight;
};
