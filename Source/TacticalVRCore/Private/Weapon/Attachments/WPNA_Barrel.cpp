// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Weapon/Attachments/WPNA_Barrel.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Component/TVRAttachPoint_Muzzle.h"
#include "Weapon/Component/TVRGunFireComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapon/Attachments/WPNA_Muzzle.h"
#include "Weapon/Component/TVRAttachPoint_Underbarrel.h"
#include "GripMotionControllerComponent.h"
#include "Grippables/HandSocketComponent.h"
#include "Weapon/Attachments/WPNA_Sight.h"

class UTVRCoreGameplaySettings;

AWPNA_Barrel::AWPNA_Barrel(const FObjectInitializer& OI) : Super(OI)
{
	FiringLocOverride = CreateDefaultSubobject<USceneComponent>(FName(TEXT("FiringLocOverride")));
	FiringLocOverride->SetupAttachment(GetStaticMeshComponent());
}

void AWPNA_Barrel::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(const auto Gun = GetGunOwner())
	{
		if( const auto Sight = Gun->GetAttachment<AWPNA_Sight>())
		{
			OnFoldSights(Sight->bFoldIronSights);
		}
	}	
}

void AWPNA_Barrel::AttachToWeapon(UTVRAttachmentPoint* AttachPoint)
{
	Super::AttachToWeapon(AttachPoint);
	
	if(GetGunOwner())
	{
		bool bAlreadyModified = false;
		TArray<UTVRAttachPoint_Muzzle*> MuzzleAPs;
		GetComponents<UTVRAttachPoint_Muzzle>(MuzzleAPs);
		if(MuzzleAPs.Num() > 0)
		{
			if(const auto Attachment = MuzzleAPs[0]->GetCurrentAttachment())
			{
				if(const auto Muzzle = Cast<AWPNA_Muzzle>(Attachment))
				{
					Muzzle->ModifyMuzzleLocation();
					bAlreadyModified = true;
				}
			}
		}
		if(!bAlreadyModified)
		{
			ModifyMuzzleLocation();
		}
	}
}

void AWPNA_Barrel::ModifyMuzzleLocation()
{
	if(GetGunOwner())
	{
		GetGunOwner()->GetFiringComponent()->SetSuppressed(bIsSuppressor);
		GetGunOwner()->GetFiringComponent()->SetWorldTransform(FiringLocOverride->GetComponentTransform(), false);
		const auto MuzzlePSC = FindComponentByClass<UParticleSystemComponent>();
		if(MuzzlePSC)
		{
			GetGunOwner()->GetFiringComponent()->MuzzleFlashOverride = MuzzlePSC;			
		}
	}
}

bool AWPNA_Barrel::GetGripSlot(const FVector& WorldLocation, UGripMotionControllerComponent* CallingController,
	FTransform& OutTransform, FName& OutSlotName) const
{
	// there might be a slot component
	const float SlotRange = 15.f; // todo: parameterize it (settings? property?)
    const float RangeSquared = SlotRange * SlotRange;
    if(const auto SecondarySlot = GetSecondarySlotComponent())
    {
    	FTransform BestTransform;
    	if(const auto SecondarySpline = Cast<USplineComponent>(SecondarySlot))
    	{
    		 BestTransform = SecondarySpline->FindTransformClosestToWorldLocation(WorldLocation, ESplineCoordinateSpace::World, false);
    	}
		else
		{
			BestTransform = SecondarySlot->GetComponentTransform();
		}
    	if((BestTransform.GetLocation() - WorldLocation).SizeSquared() <= RangeSquared)
    	{
    		OutTransform = BestTransform;
    		OutSlotName = SecondarySlot->GetFName();
    		return true;
    	}
    }
	return false;
}

UHandSocketComponent* AWPNA_Barrel::GetHandSocket_Implementation(FName SlotName) const
{
	UHandSocketComponent* HandSocket = nullptr;
	if(HandSocket && SlotName == GetPrefixedSocketName(HandSocket))
	{
		return HandSocket;
	}

    if(const auto SecondarySlot = GetSecondarySlotComponent())
    {
	    return Cast<UHandSocketComponent>(SecondarySlot);
    }
	
	return nullptr;
}

