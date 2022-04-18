// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRInternalMagazineComponent.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/TVRCartridge.h"
#include "TacticalTraceChannels.h"

UTVRInternalMagazineComponent::UTVRInternalMagazineComponent(const FObjectInitializer& OI) : Super(OI)
{
	Capacity = 8;
	CurrentAmmo = 8;

	AmmoInsertSound = nullptr;
	bUseComplexInsertion = false;
	CartridgeSpeed = 0.f;
	
}

void UTVRInternalMagazineComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(CurrentInsertingCartridge)
	{
		
	}
}


void UTVRInternalMagazineComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(CanInsertAmmo() && IsAllowedAmmo(OtherActor->GetClass()))
	{
		const auto Cartridge = Cast<ATVRCartridge>(OtherActor);
		if(Cartridge->VRGripInterfaceSettings.bIsHeld)
		{
			BeginInsertCartridge(Cartridge);
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
		if(bUseComplexInsertion)
		{
			AttachCartridge(Cartridge);
		}
		else
		{
			SetCurrentInsertCartridge(Cartridge);
			FullyInsertCartridge();
		}
	}
}

void UTVRInternalMagazineComponent::FullyInsertCartridge()
{
	if(CurrentInsertingCartridge)
	{
		InsertedAmmo.Add(CurrentInsertingCartridge->GetClass());
		CurrentInsertingCartridge->Destroy();
		
		if(MagAudioComp)
		{
			MagAudioComp->Stop();
			MagAudioComp->Play();
		}
	}
}

void UTVRInternalMagazineComponent::SetCurrentInsertCartridge(ATVRCartridge* Cartridge)
{
	CurrentInsertingCartridge = Cartridge;
}

void UTVRInternalMagazineComponent::AttachCartridge(ATVRCartridge* Cartridge)
{
	const FAttachmentTransformRules AttachRule(EAttachmentRule::KeepWorld, true);
	Cartridge->AttachToComponent(GetAttachParent(), AttachRule);
	
	Cartridge->VRGripInterfaceSettings.SlotDefaultGripType = EGripCollisionType::CustomGrip;
	Cartridge->VRGripInterfaceSettings.FreeDefaultGripType = EGripCollisionType::CustomGrip;
	Cartridge->VRGripInterfaceSettings.bSimulateOnDrop = false;	
	Cartridge->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	if(Cartridge->GetStaticMeshComponent()->IsSimulatingPhysics())
	{
		Cartridge->GetStaticMeshComponent()->SetSimulatePhysics(false);
	}
	Cartridge->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_VRTraceChannel, ECR_Overlap);
	
	if(UGripMotionControllerComponent* GrippingHand = Cartridge->VRGripInterfaceSettings.HoldingControllers[0].HoldingController)
	{		
		const FTransform SavedRelTransform = GrippingHand ? GrippingHand->GrippedObjects[0].RelativeTransform : FTransform::Identity;
		GrippingHand->DropObjectByInterface(Cartridge, 0, FVector::ZeroVector, FVector::ZeroVector);
		GrippingHand->GripObjectByInterface(Cartridge, SavedRelTransform, true, EName::NAME_None, EName::NAME_None, true);
	}
	
	SetCurrentInsertCartridge(Cartridge);
	SetComponentTickEnabled(true);
	// bCanRemoveCartridge = false;
	CartridgeSpeed = 0.f;
}
