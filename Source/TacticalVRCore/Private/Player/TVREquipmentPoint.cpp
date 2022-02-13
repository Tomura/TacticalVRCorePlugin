// This file is covered by the LICENSE file in the root of this plugin.

#include "Player/TVREquipmentPoint.h"

#include "TacticalCollisionProfiles.h"
#include "Weapon/TVRGunBase.h"


// Sets default values for this component's properties
UTVREquipmentPoint::UTVREquipmentPoint()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	SetCollisionProfileName(COLLISION_NO_COLLISION);
	PrimaryComponentTick.bCanEverTick = true;

	AttachedActor = nullptr;
	AttachRange = 40.f;
	LerpSpeed = 10.f;
	LerpRotSpeed = 10.f;
	AttachRange = 40.f;
}

void UTVREquipmentPoint::OnChildAttached(USceneComponent* ChildComponent)
{
	if(ATVRGunBase* Gun = Cast<ATVRGunBase>(ChildComponent->GetOwner()))
	{
		AttachedActor = Gun;
		Gun->SetCollisionProfile(FName("WeaponEquipped"));
	}
}

void UTVREquipmentPoint::OnChildDetached(USceneComponent* ChildComponent)
{
	if(ChildComponent->GetOwner() == AttachedActor)
	{
		if(ATVRGunBase* Gun = Cast<ATVRGunBase>(AttachedActor))
		{
			Gun->SetCollisionProfile(FName("Weapon"));
		}
		AttachedActor = nullptr;
	}
}


// Called when the game starts
void UTVREquipmentPoint::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTVREquipmentPoint::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(HasAttachment() && ShouldLerpAttachment())
	{
		const FVector RelLoc = AttachedActor->GetRootComponent()->GetRelativeLocation();
		const FRotator RelRot = AttachedActor->GetRootComponent()->GetRelativeRotation();

		FTransform TargetTransform = FTransform::Identity;		
		if(ATVRGunBase* Gun = Cast<ATVRGunBase>(AttachedActor))
		{
			const FTransform Hold2Root = Gun->GetPrimaryGripSlot()->GetRelativeTransform();
			TargetTransform = Hold2Root.Inverse();
		}
		// TargetTransform.SetScale3D(FVector::OneVector);
		const FVector TargetLoc = TargetTransform.GetLocation() / GetRelativeScale3D();
		const FVector NewRelLoc = FMath::VInterpTo(RelLoc, TargetLoc, DeltaTime, LerpSpeed);
		const FRotator NewRelRot = FMath::RInterpTo(RelRot, TargetTransform.GetRotation().Rotator(), DeltaTime, LerpRotSpeed);
		AttachedActor->GetRootComponent()->SetRelativeLocationAndRotation(NewRelLoc, NewRelRot, false);
	}
}

bool UTVREquipmentPoint::HasAttachment() const
{
	
	return AttachedActor != nullptr && AttachedActor->IsValidLowLevelFast();
}

bool UTVREquipmentPoint::ShouldLerpAttachment() const
{
	return true;
}

void UTVREquipmentPoint::AttachGun(ATVRGunBase* Gun)
{
	Gun->GetStaticMeshComponent()->SetSimulatePhysics(false);
	const bool bAttached = Gun->GetRootComponent()->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	if(bAttached)
	{		
		AttachedActor = Gun;
	}
}

bool UTVREquipmentPoint::IsPointInRange(FVector WorldLocation) const
{
	return (WorldLocation - GetComponentLocation()).SizeSquared() <= (AttachRange*AttachRange);
}

bool UTVREquipmentPoint::CanAcceptGun(ATVRGunBase* TestGun)
{
	if(AllowedGunClass && !HasAttachment())
	{
		return TestGun->IsA(AllowedGunClass) && IsPointInRange(TestGun->GetPrimaryGripSlot()->GetComponentLocation());
	}
	return false;
}


