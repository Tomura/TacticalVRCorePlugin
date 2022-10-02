// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVREjectionPort.h"

#include "TacticalCollisionProfiles.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Weapon/TVRCartridge.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/TVRSpentCartridge.h"
#include "Weapon/Component/TVRMagazineCompInterface.h"

UTVREjectionPort::UTVREjectionPort(const FObjectInitializer& OI) : Super(OI)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	
	EjectionDir = FTransform(
		FRotator(13.f, 62.f, 0.f),
		FVector(2.f, 4.f, 1.f)
	);
	InitBoxExtent(FVector(4.f, 1.5f, 1.5f));
	SetCollisionProfileName(COLLISION_MAGAZINE_INSERT, false);
	// SetGenerateOverlapEvents(true);
	
	CartridgePoolSize = 20;
	bAllowChamberload = false;
	SpentCartridgeClass = ATVRSpentCartridge::StaticClass();
	CartridgePoolIdx = -1;
	bDoNotRespawnPool = false;
}

void UTVREjectionPort::BeginPlay()
{
	Super::BeginPlay();
	PopulateCartridgePool();
	OnComponentBeginOverlap.AddDynamic(this, &UTVREjectionPort::OnBeginOverlap);
	const auto MagComp = GetOwner()->FindComponentByClass<UTVRMagazineCompInterface>();
	LinkMagComp(MagComp);

	if(EjectSound && EjectAudioComp == nullptr)
	{
		EjectAudioComp = NewObject<UAudioComponent>(this, FName(TEXT("EjectAudio")));
		EjectAudioComp->bAutoActivate = false;
		EjectAudioComp->SetSound(EjectSound);
		EjectAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void UTVREjectionPort::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if(EjectAudioComp && !EjectAudioComp->IsPendingKill())
	{
		EjectAudioComp->DestroyComponent();
		EjectAudioComp = nullptr;
	}
	
	bDoNotRespawnPool = true;
	
	for(uint8 i = 0; i < CartridgePool.Num(); i++)
	{
		auto PooledCartridge = CartridgePool[i];
	    if(PooledCartridge != nullptr && PooledCartridge->IsValidLowLevel() && !PooledCartridge->IsPendingKill()) {
		     PooledCartridge->Destroy();
	    }
		CartridgePool[i] = nullptr; // null the pointer in the pool, we don't care that much, as we will empty it later.
	}
	CartridgePool.Empty();
}


void UTVREjectionPort::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == nullptr)
	{
		return;
	}
	if(const auto Cartridge = Cast<ATVRCartridge>(OtherActor))
	{
		if(Cartridge->VRGripInterfaceSettings.bIsHeld)
		{
			TryLoadChamber(Cartridge);
		}
	}
}

void UTVREjectionPort::PopulateCartridgePool()
{
	if(SpentCartridgeClass==nullptr || CartridgePoolSize == 0 || CartridgePool.Num() > 0)
	{
		return;
	}
	
	while(CartridgePool.Num() < CartridgePoolSize) {
		ATVRSpentCartridge* NewPooledCartridge = GetWorld()->SpawnActor<ATVRSpentCartridge>(
			SpentCartridgeClass, FVector::ZeroVector, FRotator::ZeroRotator);
		NewPooledCartridge->Deactivate();
		CartridgePool.Add(NewPooledCartridge);
		NewPooledCartridge->OnDestroyed.AddDynamic(this, &UTVREjectionPort::OnPooledCartridgeDestroyed);
	}
}

ATVRSpentCartridge* UTVREjectionPort::GetCartridgeFromPool()
{
	if(CartridgePool.Num() > 0)
	{
		CartridgePoolIdx++;
		if(!CartridgePool.IsValidIndex(CartridgePoolIdx))
		{
			CartridgePoolIdx = 0;
		}
		return CartridgePool[CartridgePoolIdx];
	}
	UE_LOG(LogTemp, Error, TEXT("Could not get Cartidge from Pool. Pool is empty."));
	return nullptr;
}


FTransform UTVREjectionPort::GetEjectionDir() const
{
	return EjectionDir * this->GetComponentTransform();
}

ATVRCartridge* UTVREjectionPort::SpawnEjectedCartridge(TSubclassOf<ATVRCartridge> CartridgeClass, bool bSpent)
{
	if(CartridgeClass && !GetOwner()->IsPendingKill())
	{
		const FTransform EjectionTransform = GetEjectionDir();
		ATVRCartridge* NewCartridge = nullptr;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		const FTransform SpawnTransform = FTransform(GetComponentRotation(), EjectionTransform.GetLocation());
		if(bSpent)
		{
			// NewCartridge = GetWorld()->SpawnActor<ATVRGunCartridge>(
			//    SpentCartridgeClass, SpawnTransform, SpawnParams);
			ATVRSpentCartridge* NewSpentCartridge = GetCartridgeFromPool();
			if(NewSpentCartridge)
			{
				NewSpentCartridge->ApplySpentCartridgeSettings(CartridgeClass);
				NewSpentCartridge->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::ResetPhysics);
				NewSpentCartridge->Activate();
				NewCartridge = NewSpentCartridge;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Could not find pooled Cartridge."));
			}
		}
		else
		{
			NewCartridge = GetWorld()->SpawnActor<ATVRCartridge>(
				CartridgeClass, SpawnTransform, SpawnParams);
		}		
		
		if(NewCartridge)
		{
			const FVector EjectDir = CartridgeEjectRandomStream.VRandCone(
				EjectionTransform.TransformVector(FVector::ForwardVector), 0.35);
			const float EjectImpulse = CartridgeEjectRandomStream.RandRange(300.f, 400.f);
			const float EjectRotImpulse = CartridgeEjectRandomStream.RandRange(100.f, 250.f);
			NewCartridge->GetStaticMeshComponent()->SetSimulatePhysics(true);
			NewCartridge->GetStaticMeshComponent()->SetPhysicsLinearVelocity(EjectDir * EjectImpulse, false);
			NewCartridge->GetStaticMeshComponent()->SetPhysicsAngularVelocityInDegrees(
				EjectionTransform.TransformVector(FVector::UpVector) * EjectRotImpulse, false);

			if(EjectAudioComp)
			{
				EjectAudioComp->Stop();
				EjectAudioComp->SetBoolParameter(FName(TEXT("Insert")), false);
				EjectAudioComp->Play();
			}
			
			return NewCartridge;
		}
	}
	return nullptr;
}

void UTVREjectionPort::OnPooledCartridgeDestroyed(AActor* PooledCatridge)
{
	if(ATVRSpentCartridge* C = Cast<ATVRSpentCartridge>(PooledCatridge))
	{
		CartridgePool.Remove(C);
		if(!bDoNotRespawnPool)
		{
			// we might want to respawn more actors into our pool
			PopulateCartridgePool();
		}
	}
}

void UTVREjectionPort::LinkMagComp(UObject* MagInterface)
{
	AllowedCatridges.Empty();
	ITVRMagazineInterface::Execute_GetAllowedCatridges(MagInterface, AllowedCatridges);
}

void UTVREjectionPort::TryLoadChamber(ATVRCartridge* Cartridge)
{
	if(!bAllowChamberload)
	{
		return;
	}

	if(GetOwner())
	{
		if(const auto Gun = Cast<ATVRGunBase>(GetOwner()))
		{
			if(Gun->IsBoltOpen() && AllowedCatridges.Find(Cartridge->GetClass()) != INDEX_NONE)
			{
				if(Gun->TryChamberNewRound(Cartridge->GetClass()))
				{
					if(EjectAudioComp)
					{
						EjectAudioComp->Stop();
						EjectAudioComp->SetBoolParameter(FName(TEXT("Insert")), true);
						EjectAudioComp->Play();
					}
					Cartridge->Destroy();
				}
			}
		}
	}
}
