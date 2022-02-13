// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/TVRWeaponAttachment.h"

#include "TacticalCollisionProfiles.h"
#include "Weapon/Component/TVRAttachmentPoint.h"
#include "Components/ArrowComponent.h"
#include "Weapon/TVRGunBase.h"


FName ATVRWeaponAttachment::StaticMeshComponentName(TEXT("Mesh"));
FName ATVRWeaponAttachment::NAME_GripOverride(TEXT("AttachmentGrip"));

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
	GripSlotOverride = EName::NAME_None;
}


// Called when the game starts or when spawned
void ATVRWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();

	if(UChildActorComponent* ParentComp = GetParentComponent())
	{
		UTVRAttachmentPoint* AttachPoint = Cast<UTVRAttachmentPoint>(ParentComp);
		AttachToWeapon(AttachPoint);
	}

	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);
	for(UStaticMeshComponent* TestMesh: Meshes)
	{
		if(TestMesh->GetCollisionProfileName() == FName("Weapon"))
		{
			AttachmentMeshes.AddUnique(TestMesh);
		}
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
	return nullptr;
}

void ATVRWeaponAttachment::SetCollisionProfile(FName NewProfile)
{
	for(UStaticMeshComponent* TestMesh : AttachmentMeshes)
	{
		TestMesh->SetCollisionProfileName(NewProfile);
	}
}
