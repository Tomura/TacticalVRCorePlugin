// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Underbarrel.h"
#include "Weapon/Attachments/WPNA_ForeGrip.h"

UTVRAttachPoint_Underbarrel::UTVRAttachPoint_Underbarrel(const FObjectInitializer& OI) : Super(OI)
{
}

void UTVRAttachPoint_Underbarrel::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void UTVRAttachPoint_Underbarrel::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UTVRAttachPoint_Underbarrel, CurrentAttachmentClass))
	{
		SetChildActorClass(CurrentAttachmentClass);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UTVRAttachPoint_Underbarrel::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UTVRAttachPoint_Underbarrel, CurrentAttachmentClass))
	{
		SetChildActorClass(CurrentAttachmentClass);
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Underbarrel::GetCurrentAttachmentClass() const
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
