// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachPoint_Muzzle.h"
#include "Weapon/Attachments/WPNA_Muzzle.h"

UTVRAttachPoint_Muzzle::UTVRAttachPoint_Muzzle(const FObjectInitializer& OI) : Super(OI)
{
	CurrentAttachmentClass = nullptr;
}

#if WITH_EDITOR
void UTVRAttachPoint_Muzzle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UTVRAttachPoint_Muzzle, CurrentAttachmentClass))
	{
		SetChildActorClass(CurrentAttachmentClass);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UTVRAttachPoint_Muzzle::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UTVRAttachPoint_Muzzle, CurrentAttachmentClass))
	{
		SetChildActorClass(CurrentAttachmentClass);
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

void UTVRAttachPoint_Muzzle::BeginPlay()
{
	Super::BeginPlay();
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachPoint_Muzzle::GetCurrentAttachmentClass() const
{
	return CurrentAttachmentClass;
}

void UTVRAttachPoint_Muzzle::GetAllowedAttachments(
	TArray<TSubclassOf<ATVRWeaponAttachment>>& OutAllowedAttachments) const
{
	for(TSubclassOf<AWPNA_Muzzle> TestClass : AllowedMuzzles)
	{
		OutAllowedAttachments.Add(TestClass);
	}
}
