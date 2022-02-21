// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/TVRWeaponAttachment.h"

#include "TacticalCollisionProfiles.h"
#include "Weapon/Component/TVRAttachmentPoint.h"
#include "Components/ArrowComponent.h"
#include "Weapon/TVRGunBase.h"


FName ATVRWeaponAttachment::StaticMeshComponentName(TEXT("Mesh"));

// Sets default values
ATVRWeaponAttachment::ATVRWeaponAttachment(const FObjectInitializer& OI) : Super(OI)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(FName("RootScene"));	
	SetRootComponent(RootScene);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(ATVRWeaponAttachment::StaticMeshComponentName);
	Mesh->SetupAttachment(RootScene);
	Mesh->SetCollisionProfileName(COLLISION_WEAPON);

	SetTickGroup(TG_PostPhysics);
}


// Called when the game starts or when spawned
void ATVRWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);
	for(UStaticMeshComponent* TestMesh: Meshes)
	{
		if(TestMesh->GetCollisionProfileName() == FName("Weapon"))
		{
			AttachmentMeshes.AddUnique(TestMesh);
		}
	}
	
	GetWorldTimerManager().SetTimerForNextTick(this, &ATVRWeaponAttachment::FindAttachPointAndAttach);
}

void ATVRWeaponAttachment::OnOwnerGripped(UGripMotionControllerComponent* GrippingHand,
	const FBPActorGripInformation& GripInfo)
{
	OnOwnerGripped_Implementation(GrippingHand, GripInfo);
}

void ATVRWeaponAttachment::OnOwnerDropped(UGripMotionControllerComponent* GrippingHand,
	const FBPActorGripInformation& GripInfo, bool bSocketed)
{
	OnOwnerDropped_Implementation(GrippingHand, GripInfo, bSocketed);
}

// Called every frame
void ATVRWeaponAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATVRWeaponAttachment::FindAttachPointAndAttach()
{
	if(UChildActorComponent* ParentComp = GetParentComponent())
	{
		const auto AttachPoint = Cast<UTVRAttachmentPoint>(ParentComp);
		AttachToWeapon(AttachPoint);
	}
}

void ATVRWeaponAttachment::AttachToWeapon(UTVRAttachmentPoint* AttachPoint)
{
	if(GetGunOwner())
	{
		GetGunOwner()->EventOnGripped.RemoveAll(this);
	}
	if(AttachPoint == nullptr)
	{
		return;
	}
	ATVRGunBase* Gun = Cast<ATVRGunBase>(AttachPoint->GetOwner());
	if(Gun == nullptr)
	{
		return;
	}
	AddTickPrerequisiteActor(Gun);
	UStaticMeshComponent* GunMesh = Gun->GetStaticMeshComponent();
	const bool bWasSimulating = GunMesh->IsSimulatingPhysics();
	GunMesh->SetSimulatePhysics(false);
	if(GunMesh != nullptr)
	{
		// attach all children to its parent, for physics reasons
		TArray<USceneComponent*> MyChildren;
		GetRootComponent()->GetChildrenComponents(false, MyChildren);
		for(USceneComponent* Child: MyChildren)
		{
			Child->AttachToComponent(AttachPoint->GetAttachParent(), FAttachmentTransformRules::KeepWorldTransform);
		}
		SetOwner(Gun);
		AttachmentPoint = AttachPoint;
		AttachPoint->OnWeaponAttachmentAttached(this);	
		AddTickPrerequisiteComponent(GunMesh);
	}
	AddTickPrerequisiteActor(Gun);
	Gun->EventOnGripped.AddDynamic(this, &ATVRWeaponAttachment::OnOwnerGripped);
	Gun->EventOnDropped.AddDynamic(this, &ATVRWeaponAttachment::OnOwnerDropped);
	if(bWasSimulating)
	{
		GunMesh->SetSimulatePhysics(true);
	}
}

ATVRGunBase* ATVRWeaponAttachment::GetGunOwner() const
{
	if(GetOwner())
	{
		return Cast<ATVRGunBase>(GetOwner());
	}
	else
	{
		// there is a chance so we look for our attach parent
		if(const auto AttachParent = GetAttachParentActor())
		{
			return Cast<ATVRGunBase>(AttachParent);
		}
	}
	return nullptr;
}

void ATVRWeaponAttachment::SetCollisionProfile(FName NewProfile)
{
	for(UStaticMeshComponent* TestMesh : AttachmentMeshes)
	{
		TestMesh->SetCollisionProfileName(NewProfile);
	}
}

FName ATVRWeaponAttachment::GetPrefixedSocketName(USceneComponent* SocketComp) const
{
	if(AttachmentPoint)
	{
		const auto SocketName = SocketComp->GetFName();
		const auto NewName = FString::Printf(TEXT("%s.%s"), *AttachmentPoint->GetFName().ToString(), *SocketName.ToString());
		return FName(*NewName);
	}
	return SocketComp->GetFName();
}

void ATVRWeaponAttachment::SetVariant(uint8 Variant)
{
	SelectedVariant = Variant;
	OnVariantChanged(SelectedVariant);
}

TSubclassOf<ATVRWeaponAttachment> ATVRWeaponAttachment::GetReplacementClass_Implementation(
	ERailType RailType,
	uint8 CustomType) const
{
	return this->GetClass();
}

void ATVRWeaponAttachment::SetRailType(ERailType RailType, uint8 CustomType)
{
	OnRailTypeChanged(RailType, CustomType);
	SetVariant(SelectedVariant);
}
