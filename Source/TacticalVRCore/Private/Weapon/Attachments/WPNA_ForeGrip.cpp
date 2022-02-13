// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_ForeGrip.h"
#include "Grippables/GrippableStaticMeshComponent.h"
#include "Weapon/TVRGunBase.h"

AWPNA_ForeGrip::AWPNA_ForeGrip(const FObjectInitializer& OI) : Super(OI)
{
	GripSlotOverride = ATVRWeaponAttachment::NAME_GripOverride;

	GripSlot = CreateDefaultSubobject<USceneComponent>(FName("GripSlotComponent"));
	GripSlot->SetupAttachment(GetStaticMeshComponent());
}

bool AWPNA_ForeGrip::GetGripSlot(const FVector& WorldLocation, class UGripMotionControllerComponent* CallingController, FTransform& OutTransform, FName& OutSlotName) const
{
	if(GetPrimaryHandSocket())
	{		
		OutSlotName = FName("ForeGrip");
		OutTransform = GripSlot->GetComponentTransform();
		return true;
	}
	return false;
}

bool AWPNA_ForeGrip::IsGripped() const
{
	if(GetGunOwner())
	{
		if(const FBPActorGripInformation* GripInfo = GetGunOwner()->GetSecondaryGripInfo())
		{
			if(GripInfo->SecondaryGripInfo.SecondarySlotName == FName("ForeGrip"))
			{
				return true;
			}
		}
	}
	return false;
}

UHandSocketComponent* AWPNA_ForeGrip::GetHandSocket_Implementation(FName SlotName) const
{
	if(GetPrimaryHandSocket() && SlotName == FName("ForeGrip"))
	{
		return GetPrimaryHandSocket();
	}
	return nullptr;
}
