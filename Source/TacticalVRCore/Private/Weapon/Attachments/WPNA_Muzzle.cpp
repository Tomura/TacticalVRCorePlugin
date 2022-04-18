// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_Muzzle.h"

#include "Particles/ParticleSystemComponent.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Component/TVRGunFireComponent.h"

AWPNA_Muzzle::AWPNA_Muzzle(const FObjectInitializer& OI) : Super(OI)
{
	PrimaryActorTick.bCanEverTick = false;
	
	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(FName("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(GetStaticMeshComponent());
}

void AWPNA_Muzzle::AttachToWeapon(UTVRAttachmentPoint* AttachPoint)
{
	Super::AttachToWeapon(AttachPoint);
	
	ModifyMuzzleLocation();
}

void AWPNA_Muzzle::BeginPlay()
{
	Super::BeginPlay();
}

void AWPNA_Muzzle::Destroyed()
{
	if(GetGunOwner() && !GetGunOwner()->IsPendingKill())
	{
		if(GetGunOwner()->GetFiringComponent())
		{
			GetGunOwner()->GetFiringComponent()->ResetSuppressed();
			const auto GunCDO = GetGunOwner()->GetClass()->GetDefaultObject<ATVRGunBase>();
			const FTransform OriginalFiringLoc = GunCDO->GetFiringComponent()->GetRelativeTransform();
			GetGunOwner()->GetFiringComponent()->SetRelativeTransform(OriginalFiringLoc);
			GetGunOwner()->GetFiringComponent()->MuzzleFlashOverride = nullptr;
		}
	}
	Super::Destroyed();
}

void AWPNA_Muzzle::ModifyMuzzleLocation()
{
	if(GetGunOwner())
	{
		GetGunOwner()->GetFiringComponent()->SetSuppressed(bIsSuppressor);
		GetGunOwner()->GetFiringComponent()->SetWorldTransform(MuzzleLocation->GetComponentTransform(), false);
		const auto MuzzlePSC = FindComponentByClass<UParticleSystemComponent>();
		if(MuzzlePSC)
		{
			GetGunOwner()->GetFiringComponent()->MuzzleFlashOverride = MuzzlePSC;			
		}
	}
}
