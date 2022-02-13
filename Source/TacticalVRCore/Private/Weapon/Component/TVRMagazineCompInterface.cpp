// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRMagazineCompInterface.h"

#include "TacticalCollisionProfiles.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"

UTVRMagazineCompInterface::UTVRMagazineCompInterface(const FObjectInitializer& OI)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	
	SetCollisionProfileName(COLLISION_MAGAZINE_INSERT, false);
    ShapeColor = FColor::Turquoise;
    InitBoxExtent(FVector(3.f, 1.f, 5.f));
}

void UTVRMagazineCompInterface::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UTVRMagazineCompInterface::OnBeginOverlap);
}

ATVRGunBase* UTVRMagazineCompInterface::GetGunOwner() const
{
	if(GetOwner())
	{
		if(auto Gun = Cast<ATVRGunBase>(GetOwner()))
		{
			return Gun;
		}
		if(auto Attachment = Cast<ATVRWeaponAttachment>(GetOwner()))
		{
			return Attachment->GetGunOwner();
		}
	}
	return nullptr;
}

void UTVRMagazineCompInterface::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void UTVRMagazineCompInterface::OnOwnerGripReleased(ATVRCharacter* OwningChar, class UGripMotionControllerComponent*)
{
}
