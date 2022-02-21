// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRAttachmentPoint.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "Weapon/TVRGunBase.h"


// Sets default values for this component's properties
UTVRAttachmentPoint::UTVRAttachmentPoint(const FObjectInitializer& OI) : Super(OI)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	CurrentAttachment = nullptr;
	PreferredVariant = 0;
	RailType = ERailType::Picatinny;
	CustomRailType = 0;
}

void UTVRAttachmentPoint::OnWeaponAttachmentAttached(ATVRWeaponAttachment* NewAttachment)
{
	CurrentAttachment = NewAttachment;
	if(EventOnWeaponAttachmentAttached.IsBound())
	{
		EventOnWeaponAttachmentAttached.Broadcast(this, NewAttachment);
	}
}

TSubclassOf<ATVRWeaponAttachment> UTVRAttachmentPoint::GetCurrentAttachmentClass() const
{
	const auto AttachClass = GetCurrentAttachmentClass_Internal();
	if(AttachClass)
	{
		return GetDefault<ATVRWeaponAttachment>(AttachClass)->GetReplacementClass(RailType, CustomRailType);
	}
	return AttachClass;
}

// Called when the game starts
void UTVRAttachmentPoint::BeginPlay()
{
	Super::BeginPlay();
}

void UTVRAttachmentPoint::OnRegister()
{
	Super::OnRegister();
}

void UTVRAttachmentPoint::OnConstruction()
{
	if(GetCurrentAttachmentClass() && GetCurrentAttachmentClass() != GetChildActorClass())
	{
		SetChildActorClass( GetCurrentAttachmentClass());
	}
	else if (GetCurrentAttachmentClass() == nullptr) // firsts condition fails on null
	{
		SetChildActorClass(nullptr);
	}
	
	if(!IsTemplate() && GetChildActor()) // this can create a double execution of setPreferredVariant, but it should be ok
	{
		if(const auto WPNA = Cast<ATVRWeaponAttachment>(GetChildActor()))
		{
			WPNA->SetVariant(PreferredVariant);
			WPNA->SetRailType(RailType, CustomRailType);
		}
	}
}

void UTVRAttachmentPoint::CreateChildActor()
{
	Super::CreateChildActor();
		
	if(!IsTemplate() && GetChildActor())
	{
		if(auto WPNA = Cast<ATVRWeaponAttachment>(GetChildActor()))
		{
			WPNA->SetVariant(PreferredVariant);
			WPNA->SetRailType(RailType, CustomRailType);
		}
	}
}


bool UTVRAttachmentPoint::SetCurrentAttachmentClass(TSubclassOf<ATVRWeaponAttachment> NewClass)
{
	return false;
}

// Called every frame
void UTVRAttachmentPoint::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

ATVRGunBase* UTVRAttachmentPoint::GetGunOwner() const
{
	return GetOwner() ? Cast<ATVRGunBase>(GetOwner()) : nullptr;
}

