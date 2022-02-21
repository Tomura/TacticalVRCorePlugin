// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_ForeGrip.h"
#include "Grippables/GrippableStaticMeshComponent.h"
#include "Weapon/TVRGunBase.h"

AWPNA_ForeGrip::AWPNA_ForeGrip(const FObjectInitializer& OI) : Super(OI)
{
	GripSlot = CreateDefaultSubobject<USceneComponent>(FName("GripSlotComponent"));
	GripSlot->SetupAttachment(GetStaticMeshComponent());
	PrimarySlotGripDistance = 15.f;
}

bool AWPNA_ForeGrip::GetGripSlot(const FVector& WorldLocation, class UGripMotionControllerComponent* CallingController, FTransform& OutTransform, FName& OutSlotName) const
{
	if(GetPrimaryHandSocket())
	{
		OutTransform = GetPrimaryHandSocket()->GetComponentTransform();

		const float GripDistanceSq = FVector::DistSquared(OutTransform.GetLocation(), WorldLocation);
		const float AllowedDistanceSq = PrimarySlotGripDistance*PrimarySlotGripDistance;
		if(GripDistanceSq <= AllowedDistanceSq)
		{
			OutSlotName = GetPrefixedSocketName(GetPrimaryHandSocket());
			return true;
		}
	}
	return false;
}

bool AWPNA_ForeGrip::IsGripped() const
{
	if(GetGunOwner())
	{
		if(const FBPActorGripInformation* GripInfo = GetGunOwner()->GetSecondaryGripInfo())
		{
			if(GripInfo->SecondaryGripInfo.SecondarySlotName == GetPrefixedSocketName(GetPrimaryHandSocket()))
			{
				return true;
			}
		}
	}
	return false;
}

UHandSocketComponent* AWPNA_ForeGrip::GetHandSocket_Implementation(FName SlotName) const
{
	if(GetPrimaryHandSocket() && SlotName == GetPrefixedSocketName(GetPrimaryHandSocket()))
	{
		return GetPrimaryHandSocket();
	}
	return nullptr;
}
