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

void ATVRWeaponAttachment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitAttachments();
}


// Called when the game starts or when spawned
void ATVRWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();
	InitAttachments();
	
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

void ATVRWeaponAttachment::InitAttachments()
{
	// if there are attachment points we might need to call their construction logic
	TArray<UTVRAttachmentPoint*> AttachPoints;
	GetComponents<UTVRAttachmentPoint>(AttachPoints);
	for(UTVRAttachmentPoint* LoopPoint: AttachPoints)
	{
		LoopPoint->OnConstruction();
	}
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
	ATVRGunBase* Gun = AttachPoint->GetGunOwner();
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
	const auto ParentActor = GetOwner() ? GetOwner() : GetAttachParentActor();
	if(ParentActor)
	{
		if(const auto ParentAttachment = Cast<ATVRWeaponAttachment>(ParentActor))
		{
			return ParentAttachment->GetGunOwner();
		}
		return Cast<ATVRGunBase>(ParentActor);
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
	NativeOnVariantChanged(SelectedVariant, SelectedColor);
}

void ATVRWeaponAttachment::SetColorVariant(uint8 Variant)
{
	SelectedColor = Variant;
	NativeOnVariantChanged(SelectedVariant, SelectedColor);
}

void ATVRWeaponAttachment::NativeOnVariantChanged(uint8 Variant, uint8 ColorVariant)
{
	OnVariantChanged(Variant, ColorVariant);
	InitAttachments();
}

void ATVRWeaponAttachment::NativeOnRailTypeChanged(ETVRRailType RailType, uint8 CustomType)
{
	OnRailTypeChanged(RailType, CustomType);
	NativeOnVariantChanged(SelectedVariant, SelectedColor);
}

TSubclassOf<ATVRWeaponAttachment> ATVRWeaponAttachment::GetReplacementClass_Implementation(
	ETVRRailType RailType,
	uint8 CustomType) const
{
	return this->GetClass();
}

void ATVRWeaponAttachment::SetRailType(ETVRRailType RailType, uint8 CustomType)
{
	NativeOnRailTypeChanged(RailType, CustomType);
}