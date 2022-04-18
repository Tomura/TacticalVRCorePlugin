// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "Weapon/Component/TVRLoadableBreechComponent.h"

#include "TacticalTraceChannels.h"
#include "TacticalCollisionProfiles.h"
#include "Components/AudioComponent.h"
#include "Weapon/TVRCartridge.h"
#include "Weapon/Component/TVRGunFireComponent.h"

#define MAG_AUDIO_StartInsert 0
#define MAG_AUDIO_FullyInserted 1
#define MAG_AUDIO_StartDrop 2
#define MAG_AUDIO_FullyDropped 3

ULoadableBreechComponent::ULoadableBreechComponent(const FObjectInitializer& OI) : Super(OI)
{
	Progress = 0.f;
	CurrentCartridge = nullptr;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	Distance = 12.f;
	bCanRemoveCartridge = false;
	bCanFullyInsertCartridge = false;
	BreechState = ETVRLoadableBreechState::Closed;
	BreechOpenTime = 0.5f;
	bReleaseCartridgeWhenOpened = false;
	EjectorForce = 0.f;
}

void ULoadableBreechComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Distance < 0.1f)
	{
		Distance = 0.1f;
	}
	if(!GetFiringComp())
	{
		if(const auto FoundComp = GetOwner()->GetComponentByClass(UTVRGunFireComponent::StaticClass()))
		{
			AssignFiringComp(Cast<UTVRGunFireComponent>(FoundComp));
		}
	}

	if(CartridgeInsertSound)
	{
		CartridgeInsertAudioComp = NewObject<UAudioComponent>(this);
		CartridgeInsertAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CartridgeInsertAudioComp->SetSound(CartridgeInsertSound);
		CartridgeInsertAudioComp->SetAutoActivate(false);
	}
	
	if(OpenCloseSound)
	{
		OpenCloseAudioComp = NewObject<UAudioComponent>(this);
		OpenCloseAudioComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		OpenCloseAudioComp->SetSound(OpenCloseSound);
		OpenCloseAudioComp->SetAutoActivate(false);
	}
}

void ULoadableBreechComponent::BeginDestroy()
{
	if(GetCurrentCartridge() && !GetCurrentCartridge()->IsPendingKill())
	{
		GetCurrentCartridge()->OnGripped.RemoveDynamic(this, &ULoadableBreechComponent::OnCartridgeGrabbed);
		GetCurrentCartridge()->OnDropped.RemoveDynamic(this, &ULoadableBreechComponent::OnCartridgeDropped);
		GetCurrentCartridge()->Destroy();
	}
	if(GetFiringComp() && !GetFiringComp()->IsPendingKill())
	{
		GetFiringComp()->OnCartridgeSpent.RemoveDynamic(this, &ULoadableBreechComponent::OnCartridgeSpent);
	}
	if(GetCartridgeInsertAudioComp() && !GetCartridgeInsertAudioComp()->IsPendingKill())
	{
		GetCartridgeInsertAudioComp()->DestroyComponent();
	}
	Super::BeginDestroy();
}



void ULoadableBreechComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		
	if(GetCurrentCartridge())
	{
		const FTransform ParentTransform = GetAttachParent()->GetComponentTransform();
		const FTransform RelT = GetRelativeTransform();
		const FVector TargetLoc = (TargetTransform * RelT * ParentTransform).GetLocation();
		const FVector ZeroLoc = TargetLoc - Distance * GetForwardVector();
		
		if(GetCurrentCartridge()->VRGripInterfaceSettings.bIsHeld)
		{			
			UGripMotionControllerComponent* GrippingHand = GetCurrentCartridge()->VRGripInterfaceSettings.HoldingControllers[0].HoldingController;
			const FTransform HandTransform = GrippingHand->GetPivotTransform();
			const auto HandSocket = ITVRHandSocketInterface::Execute_GetHandSocket(GetCurrentCartridge(), NAME_None);
			const FTransform SocketTransform = HandSocket ? HandSocket->GetRelativeTransform() : FTransform::Identity;
			const FVector ClosestPointUnlimited = FMath::ClosestPointOnInfiniteLine(TargetLoc, ZeroLoc, (SocketTransform.Inverse() * HandTransform).GetLocation());
			const float ProgressUnlimited = GetComponentTransform().InverseTransformVector(ClosestPointUnlimited - ZeroLoc).X / Distance;	

			if(!bCanRemoveCartridge &&  ProgressUnlimited > 0.f)
			{
				bCanRemoveCartridge = true;
			}
			if(!bCanFullyInsertCartridge && ProgressUnlimited < 1 - KINDA_SMALL_NUMBER)
			{
				bCanFullyInsertCartridge = true;
			}
			
			Progress = FMath::Clamp(ProgressUnlimited, 0.f, 1.f);
			
			GetCurrentCartridge()->SetActorRelativeRotation((RelT * TargetTransform).Rotator(), false, nullptr, ETeleportType::ResetPhysics);
			GetCurrentCartridge()->SetActorLocation(ZeroLoc + Progress * Distance * GetForwardVector(),
				false, nullptr, ETeleportType::ResetPhysics);
			
			bCartridgeIsLocked = false;
			CartridgeSpeed = 0.f;
			if(bCanFullyInsertCartridge && 1.f - ProgressUnlimited < KINDA_SMALL_NUMBER)
			{
				FullyInsertCartridge();
			}
			else if(bCanRemoveCartridge && ProgressUnlimited < 0.f)
			{				
				FullyRemoveCartridge();
			}
		}
		else if (!bCartridgeIsLocked)
		{
			const FVector Gravity = GetWorld()->GetGravityZ() * FVector::UpVector;
			float Acceleration = GetForwardVector() | Gravity;
			if(bReleaseCartridgeWhenOpened)
			{
				Acceleration -= EjectorForce;
			}
			CartridgeSpeed += Acceleration * DeltaTime;
			Progress = FMath::Clamp(Progress + CartridgeSpeed * DeltaTime / Distance, 0.f, 1.f);
			
			GetCurrentCartridge()->SetActorLocation(ZeroLoc + Progress * Distance * GetForwardVector(),
				false, nullptr, ETeleportType::ResetPhysics);
			
			if(Progress <= 0.f)
			{				
				FullyRemoveCartridge();
			}
			else if (Progress >= 1.f)
			{
				FullyInsertCartridge();
			}
		}
	}
}

void ULoadableBreechComponent::SetMagazineCollisionProfile(FName NewProfile)
{
	Super::SetMagazineCollisionProfile(NewProfile);
}

bool ULoadableBreechComponent::CanBoltLock() const
{
	return false;
}

bool ULoadableBreechComponent::IsEmpty() const
{
	return CurrentCartridge == nullptr;
}

float ULoadableBreechComponent::GetAmmoInsertProgress()
{
	return GetCurrentCartridge() ? Progress : 0.f;
}


void ULoadableBreechComponent::OnOwnerGripReleased(ATVRCharacter* OwningChar,
	UGripMotionControllerComponent* ReleasingHand)
{
	Super::OnOwnerGripReleased(OwningChar, ReleasingHand);
}

bool ULoadableBreechComponent::CanFeedAmmo() const
{
	return GetCurrentCartridge() != nullptr;
}

TSubclassOf<ATVRCartridge> ULoadableBreechComponent::TryFeedAmmo()
{
	return GetCurrentCartridge()->GetClass();
}

void ULoadableBreechComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATVRGunBase* Gun = GetGunOwner();
	if(Gun == nullptr)
	{
		return;
	}
	if(BreechState == ETVRLoadableBreechState::Open && GetCurrentCartridge() == nullptr)
	{
		if(const auto InsertedCartridge = Cast<ATVRCartridge>(OtherActor))
		{
			if(IsAllowedAmmo(InsertedCartridge->GetClass()))
			{
				BeginInsertCartridge(InsertedCartridge);
			}
		}
	}
}

bool ULoadableBreechComponent::OpenBreech()
{
	if((Progress == 0.f || Progress == 1.f) && BreechState==ETVRLoadableBreechState::Closed)
	{
		BreechState = ETVRLoadableBreechState::Opening;
		GetWorld()->GetTimerManager().SetTimer(BreechOpenTimer,
			this, &ULoadableBreechComponent::OnBreechOpened, GetOpenDuration(), false);		
		EventOnBeginOpenBreech.Broadcast();
		if(GetFiringComp())
		{
			GetFiringComp()->TryEjectCartridge();
		}
		
		if(GetOpenCloseAudioComp())
		{
			GetOpenCloseAudioComp()->Stop();
			GetOpenCloseAudioComp()->SetBoolParameter(FName("Open"), true);
			GetOpenCloseAudioComp()->Play();
		}

		
		return true;
	}
	return false;
}

void ULoadableBreechComponent::OnBreechOpened()
{
	BreechState = ETVRLoadableBreechState::Open;
	if(GetCurrentCartridge())
	{
		GetCurrentCartridge()->VRGripInterfaceSettings.bDenyGripping = false;
		
		if(bReleaseCartridgeWhenOpened)
		{
			bCartridgeIsLocked = false;
			SetComponentTickEnabled(true);
		}
		
	}
	EventOnOpenedBreech.Broadcast();
}

void ULoadableBreechComponent::OnBreechClosed()
{
	BreechState = ETVRLoadableBreechState::Closed;
	EventOnClosedBreech.Broadcast();

	if(GetCurrentCartridge() && GetFiringComp() && !GetCurrentCartridge()->bIsSpent)
	{
		GetFiringComp()->TryLoadCartridge(GetCurrentCartridge()->GetClass());
	}
}

bool ULoadableBreechComponent::CloseBreech()
{
	if((Progress == 0.f || Progress == 1.f) && BreechState==ETVRLoadableBreechState::Open)
	{
		BreechState = ETVRLoadableBreechState::Closing;
		if(GetCurrentCartridge())
		{		
			GetCurrentCartridge()->VRGripInterfaceSettings.bDenyGripping = true;
		}
		GetWorld()->GetTimerManager().SetTimer(BreechOpenTimer,
			this, &ULoadableBreechComponent::OnBreechClosed, GetOpenDuration(), false);
		EventOnBeginCloseBreech.Broadcast();
		if(GetOpenCloseAudioComp())
		{
			GetOpenCloseAudioComp()->Stop();
			GetOpenCloseAudioComp()->SetBoolParameter(FName("Open"), false);
			GetOpenCloseAudioComp()->Play();
		}
		
		return true;
	}
	return false;
}

void ULoadableBreechComponent::AssignFiringComp(UTVRGunFireComponent* NewFiringComp)
{
	if(GetFiringComp()) // remove any old delegates
	{
		GetFiringComp()->OnCartridgeSpent.RemoveDynamic(this, &ULoadableBreechComponent::OnCartridgeSpent);
	}
	FiringComp = NewFiringComp;
	if(GetFiringComp()) // only do this if we actually assigned a valid object
	{
		GetFiringComp()->OnCartridgeSpent.AddDynamic(this, &ULoadableBreechComponent::OnCartridgeSpent);
	}
}

bool ULoadableBreechComponent::IsAllowedAmmo(TSubclassOf<ATVRCartridge> CartridgeClass) const
{
	return AllowedCartridges.Find(CartridgeClass) != INDEX_NONE;
}

void ULoadableBreechComponent::BeginInsertCartridge(ATVRCartridge* CartridgeToInsert)
{
	if(CartridgeToInsert->VRGripInterfaceSettings.bIsHeld)
	{		
		AttachCartridge(CartridgeToInsert);
		if(GetCartridgeInsertAudioComp())
		{
			GetCartridgeInsertAudioComp()->Stop();
			GetCartridgeInsertAudioComp()->SetIntParameter(FName("MagEvent"), MAG_AUDIO_StartInsert);
			GetCartridgeInsertAudioComp()->Play();
		}
	}
}

void ULoadableBreechComponent::OnCartridgeSpent()
{
	if(GetCurrentCartridge())
	{
		GetCurrentCartridge()->bIsSpent = true;
		GetCurrentCartridge()->GetStaticMeshComponent()->SetStaticMesh(GetCurrentCartridge()->GetSpentCartridgeMesh());
	}
}

void ULoadableBreechComponent::OnCartridgeGrabbed(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	SetComponentTickEnabled(true);
	if(GetCartridgeInsertAudioComp())
	{
		GetCartridgeInsertAudioComp()->Stop();
		GetCartridgeInsertAudioComp()->SetIntParameter(FName("MagEvent"), MAG_AUDIO_StartDrop);
		GetCartridgeInsertAudioComp()->Play();
	}
}

void ULoadableBreechComponent::OnCartridgeDropped(UGripMotionControllerComponent * GrippingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	if(Progress >= 1.f)
	{		
		FullyInsertCartridge();
	}
}

void ULoadableBreechComponent::SetCurrentCartridge(ATVRCartridge* NewCartridge)
{
	if(CurrentCartridge)
	{
		CurrentCartridge->OnGripped.RemoveDynamic(this, &ULoadableBreechComponent::OnCartridgeGrabbed);
		CurrentCartridge->OnDropped.RemoveDynamic(this, &ULoadableBreechComponent::OnCartridgeDropped);
	}
	CurrentCartridge = NewCartridge;
	
	if(CurrentCartridge)
	{
		CurrentCartridge->OnGripped.AddDynamic(this, &ULoadableBreechComponent::OnCartridgeGrabbed);
		CurrentCartridge->OnDropped.AddDynamic(this, &ULoadableBreechComponent::OnCartridgeDropped);
	}
}

void ULoadableBreechComponent::AttachCartridge(ATVRCartridge* Cartridge)
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
	
	SetCurrentCartridge(Cartridge);
	SetComponentTickEnabled(true);
	bCanRemoveCartridge = false;
	CartridgeSpeed = 0.f;
}

ATVRCartridge* ULoadableBreechComponent::DetachCartridge()
{
	const auto PrevCartridge = GetCurrentCartridge();
	SetCurrentCartridge(nullptr);
	
	const auto CartridgeCDO = GetDefault<ATVRCartridge>(PrevCartridge->GetClass());
	PrevCartridge->VRGripInterfaceSettings.SlotDefaultGripType = CartridgeCDO->VRGripInterfaceSettings.SlotDefaultGripType;
	PrevCartridge->VRGripInterfaceSettings.FreeDefaultGripType = CartridgeCDO->VRGripInterfaceSettings.FreeDefaultGripType;
	PrevCartridge->VRGripInterfaceSettings.bSimulateOnDrop = true;
	PrevCartridge->GetStaticMeshComponent()->SetCollisionProfileName(COLLISION_WEAPON);

	if(PrevCartridge->VRGripInterfaceSettings.bIsHeld)
	{
		if(UGripMotionControllerComponent* GrippingHand = PrevCartridge->VRGripInterfaceSettings.HoldingControllers[0].HoldingController)
		{
			const FTransform RelTransform = GrippingHand->GrippedObjects[0].RelativeTransform; 
			GrippingHand->DropObjectByInterface(PrevCartridge,
				0, FVector::ZeroVector, FVector::ZeroVector);
			GrippingHand->GripObjectByInterface(PrevCartridge, RelTransform, true,
				EName::NAME_None, EName::NAME_None, true);
		}
	}
	else
	{
		PrevCartridge->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		PrevCartridge->GetStaticMeshComponent()->SetSimulatePhysics(true);
		PrevCartridge->GetStaticMeshComponent()->AddImpulse(GetForwardVector() * CartridgeSpeed, NAME_None, true);
	}
	return PrevCartridge;
}

void ULoadableBreechComponent::FullyInsertCartridge()
{
	Progress = 1.f;
	GetCurrentCartridge()->SetActorRelativeTransform(TargetTransform * GetRelativeTransform(),
		false, nullptr, ETeleportType::ResetPhysics);
	
	if(GetCurrentCartridge()->VRGripInterfaceSettings.bIsHeld)
	{
		if(UGripMotionControllerComponent* GrippingHand = GetCurrentCartridge()->VRGripInterfaceSettings.HoldingControllers[0].HoldingController)
		{
			GrippingHand->DropObjectByInterface(GetCurrentCartridge(), 0,
				FVector::ZeroVector, FVector::ZeroVector);
		}
	}
	
	bCanFullyInsertCartridge = false;
	bCartridgeIsLocked = true;
	
	SetComponentTickEnabled(false);
	
	if(GetCartridgeInsertAudioComp())
	{
		GetCartridgeInsertAudioComp()->Stop();
		GetCartridgeInsertAudioComp()->SetIntParameter(FName("MagEvent"), MAG_AUDIO_FullyInserted);
		GetCartridgeInsertAudioComp()->Play();
	}
}

void ULoadableBreechComponent::FullyRemoveCartridge()
{
	Progress = 0.f;
	DetachCartridge();
	SetComponentTickEnabled(false);
	if(GetCartridgeInsertAudioComp())
	{
		GetCartridgeInsertAudioComp()->Stop();
		GetCartridgeInsertAudioComp()->SetIntParameter(FName("MagEvent"), MAG_AUDIO_FullyDropped);
		GetCartridgeInsertAudioComp()->Play();
	}
}
