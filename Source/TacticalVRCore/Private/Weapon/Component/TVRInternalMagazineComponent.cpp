// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRInternalMagazineComponent.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/TVRCartridge.h"

UTVRInternalMagazineComponent::UTVRInternalMagazineComponent(const FObjectInitializer& OI) : Super(OI)
{
	Capacity = 8;
	CurrentAmmo = 8;

	AmmoInsertSound = nullptr;
}


void UTVRInternalMagazineComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(CanInsertAmmo())
	{
		if(IsAllowedAmmo(OtherActor->GetClass()))
		{
			class ATVRCartridge* Cartridge = Cast<ATVRCartridge>(OtherActor);
			if(Cartridge->VRGripInterfaceSettings.bIsHeld)
			{
				BeginInsertCartridge(Cartridge);
			}
		}
	}
}

bool UTVRInternalMagazineComponent::IsAllowedAmmo(UClass* TestClass) const
{
	return CompatibleAmmo.Find(TestClass) != INDEX_NONE;
}

bool UTVRInternalMagazineComponent::CanInsertAmmo() const
{
	if(!IsFull())
	{
		return true;
	}
	return false;
}

bool UTVRInternalMagazineComponent::IsFull() const
{
	return InsertedAmmo.Num() >= Capacity;
}

void UTVRInternalMagazineComponent::BeginPlay()
{
	Super::BeginPlay();

	if(AmmoInsertSound && !MagAudioComp)
	{
		MagAudioComp = NewObject<UAudioComponent>(GetOwner());
		MagAudioComp->bAutoActivate = false;
		MagAudioComp->SetSound(AmmoInsertSound);
		MagAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

bool UTVRInternalMagazineComponent::IsEmpty() const
{
	return InsertedAmmo.Num() <= 0;
}

bool UTVRInternalMagazineComponent::CanFeedAmmo() const
{
	return !IsEmpty();
}

TSubclassOf<ATVRCartridge> UTVRInternalMagazineComponent::TryFeedAmmo()
{
	if(CanFeedAmmo())
	{
		TSubclassOf<ATVRCartridge> ReturnCartridge = InsertedAmmo.Last();
		InsertedAmmo.RemoveAt(InsertedAmmo.Num() - 1);
		return ReturnCartridge;
	}
	return nullptr;
}

bool UTVRInternalMagazineComponent::CanBoltLock() const
{
	return IsEmpty();
}

void UTVRInternalMagazineComponent::BeginInsertCartridge(ATVRCartridge* Cartridge)
{
	if(Cartridge)
	{
		InsertedAmmo.Add(Cartridge->GetClass());
		Cartridge->Destroy();
		
		if(MagAudioComp)
		{
			MagAudioComp->Stop();
			MagAudioComp->Play();
		}
	}
}
