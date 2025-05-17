// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Inventory/TVRAmmoSlot.h"

#include "TacticalCollisionProfiles.h"
#include "Weapon/TVRMagazine.h"

UTVRAmmoSlot::UTVRAmmoSlot() : Super()
{
	Capacity = 2;
	AllowedMagazineClass = nullptr;
	
}

bool UTVRAmmoSlot::CanAcceptMagazine(ATVRMagazine* Mag) const
{
	return (Mag->GetActorLocation() - GetComponentLocation()).SizeSquared() < 100.f
	&& Magazines.Num() < Capacity
	&& !Magazines.Find(Mag);
}

void UTVRAmmoSlot::AddMagazine(ATVRMagazine* Mag)
{
	if(CanAcceptMagazine(Mag))
	{
		Mag->GetStaticMeshComponent()->SetSimulatePhysics(false);
		Mag->SetCollisionProfile(COLLISION_NO_COLLISION);
		Mag->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Mag->OnDestroyed.AddDynamic(this, &UTVRAmmoSlot::OnMagDestroyed);

	}
}

void UTVRAmmoSlot::RemoveMagazine(ATVRMagazine* Mag)
{
	if(Magazines.Find(Mag))
	{
		Mag->OnDestroyed.RemoveDynamic(this, &UTVRAmmoSlot::OnMagDestroyed);
		Magazines.Remove(Mag);		
		Mag->GetStaticMeshComponent()->SetSimulatePhysics(false);
		Mag->SetCollisionProfile(COLLISION_NO_COLLISION);
	}
}

void UTVRAmmoSlot::OnMagDestroyed(AActor* DestroyedActor)
{
	DestroyedActor->OnDestroyed.RemoveDynamic(this, &UTVRAmmoSlot::OnMagDestroyed);
	if(const auto Mag = Cast<ATVRMagazine>(DestroyedActor))
	{
		Magazines.Remove(Mag);
	}
}

void UTVRAmmoSlot::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);
}

void UTVRAmmoSlot::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
}
