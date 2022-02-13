// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRGunWithChild.h"
#include "Components/TVRHoverInputVolume.h"
#include "Weapon/Component/TVRTriggerComponent.h"


ATVRGunWithChild::ATVRGunWithChild(const FObjectInitializer& OI) : Super(OI)
{
	ChildGun = nullptr;
}

void ATVRGunWithChild::BeginPlay()
{
	Super::BeginPlay();

	if(GetChildHoverInputComp())
	{
		GetChildHoverInputComp()->EventOnMagReleasePressed.AddDynamic(this, &ATVRGunWithChild::OnMagReleasePressedHovered);
	}
}

void ATVRGunWithChild::Destroyed()
{
	Super::Destroyed();
	OnStopFire();
	if(ChildGun)
	{
		ChildGun->OnDestroyed.RemoveDynamic(this, &ATVRGunWithChild::OnChildGunDestroyed);
		ChildGun->Destroy();
	}
}

void ATVRGunWithChild::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if(ChildGun)
	{		
		ChildGun->SetOwner(NewOwner);
	}
}

void ATVRGunWithChild::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController,
	const FBPActorGripInformation& GripInformation)
{
	Super::OnGrip_Implementation(GrippingController, GripInformation);
	if(ChildGun && ChildGun->GetTriggerComponent())
	{
		ChildGun->GetTriggerComponent()->ActivateTrigger(GrippingController);
	}
}

void ATVRGunWithChild::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController,
	const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	Super::OnGripRelease_Implementation(ReleasingController, GripInformation, bWasSocketed);
	if(ChildGun && ChildGun->GetTriggerComponent())
	{
		ChildGun->GetTriggerComponent()->DeactivateTrigger();
	}
}


bool ATVRGunWithChild::CanStartFire() const
{
	return IsGrippedAtPrimaryGrip();
}

void ATVRGunWithChild::OnMagReleasePressedHovered(UGripMotionControllerComponent* Controller)
{
	if(ChildGun && ChildGun->IsValidLowLevelFast())
	{
		ChildGun->OnMagReleasePressed();		
		GetWorldTimerManager().SetTimer(MagReleaseTimer, this, &ATVRGunWithChild::OnMagReleaseReleasedHovered, 0.5f, false);
	}
}

void ATVRGunWithChild::OnMagReleaseReleasedHovered()
{
	if(ChildGun && ChildGun->IsValidLowLevelFast())
	{
		ChildGun->OnMagReleaseReleased();		
	}
}

void ATVRGunWithChild::SetChildGun(ATVRGunBase* NewChild)
{
	if(ChildGun != nullptr)
	{
		ChildGun->OnDestroyed.RemoveDynamic(this, &ATVRGunWithChild::OnChildGunDestroyed);
	}
	
	ChildGun = NewChild;
	
	if(ChildGun)
	{
		ChildGun->OnDestroyed.AddDynamic(this, &ATVRGunWithChild::OnChildGunDestroyed);
	}
}

void ATVRGunWithChild::AddRecoil()
{
	// This gun cannot add recoil of its own, but rather it's child gun will cause recoil
	// it will be added as a custom recoil
}


void ATVRGunWithChild::OnChildGunDestroyed(AActor* DestroyedActor)
{
	ChildGun = nullptr;
}


