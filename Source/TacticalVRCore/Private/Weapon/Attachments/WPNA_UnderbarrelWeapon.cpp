// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Attachments/WPNA_UnderbarrelWeapon.h"

#include "TacticalCollisionProfiles.h"
#include "Player/TVRCharacter.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Component/TVRGunFireComponent.h"
#include "Weapon/Component/TVRTriggerComponent.h"


FName AWPNA_UnderbarrelWeapon::PrimaryGripName(TEXT("UnderbarrelPrimary"));

// Sets default values
AWPNA_UnderbarrelWeapon::AWPNA_UnderbarrelWeapon(const FObjectInitializer& OI) : Super(OI)
{
	Movables = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("Movables")));
	Movables->SetupAttachment(GetStaticMeshComponent());
	Movables->SetCollisionProfileName(COLLISION_NO_COLLISION);

	SecondaryGripSlot = CreateDefaultSubobject<USceneComponent>(FName(TEXT("SecondaryGripSlotComponent")));
	SecondaryGripSlot->SetupAttachment(GetStaticMeshComponent());

	TriggerComponent = CreateDefaultSubobject<UTVRTriggerComponent>(FName(TEXT("Trigger")));

	FiringComponent = CreateDefaultSubobject<UTVRGunFireComponent>(FName("FiringComponent"));
	FiringComponent->SetupAttachment(GetStaticMeshComponent());
	
    bMagReleasePressed = true;

	PrimaryActorTick.bCanEverTick = true;
}

void AWPNA_UnderbarrelWeapon::BeginPlay()
{
	Super::BeginPlay();
	TriggerComponent->OnTriggerActivate.AddDynamic(this, &AWPNA_UnderbarrelWeapon::OnStartFire);
	TriggerComponent->OnTriggerActivate.AddDynamic(this, &AWPNA_UnderbarrelWeapon::OnStopFire);
	FiringComponent->OnFire.AddDynamic(this, &AWPNA_UnderbarrelWeapon::OnFire);
}

bool AWPNA_UnderbarrelWeapon::GetGripSlot (
	const FVector& WorldLocation, 
	class UGripMotionControllerComponent* CallingController,
	FTransform& OutTransform,
	FName& OutSlotName
) const
{
	const float MaxDistance = 10.f;
	check(GetPrimaryHandSocket());
	const FTransform PrimaryTransform = GetPrimaryHandSocket()->GetHandSocketTransform(CallingController);
	if((WorldLocation - PrimaryTransform.GetLocation()).SizeSquared() <= MaxDistance * MaxDistance)
	{		
		Super::GetGripSlot(WorldLocation, CallingController, OutTransform, OutSlotName);
		OutSlotName = PrimaryGripName;
	}
	else
	{
		OutTransform = SecondaryGripSlot->GetComponentTransform();
		OutSlotName = FName(TEXT("UnderbarrelSecondary"));
	}
	return true;
}

UHandSocketComponent* AWPNA_UnderbarrelWeapon::GetHandSocket_Implementation(FName SlotName) const
{
	if(SlotName == PrimaryGripName)
	{
		return GetPrimaryHandSocket();
	}
	return nullptr;
}

void AWPNA_UnderbarrelWeapon::OnGripped(UGripMotionControllerComponent* GrippingController,
	const FBPActorGripInformation& GripInformation, bool bIsSecondaryGrip)
{
	Super::OnGripped(GrippingController, GripInformation, bIsSecondaryGrip);
	TriggerComponent->ActivateTrigger(GrippingController);
}

void AWPNA_UnderbarrelWeapon::OnReleased(UGripMotionControllerComponent* ReleasingController,
	const FBPActorGripInformation& GripInformation, bool bIsSecondaryGrip)
{
	Super::OnGripped(ReleasingController, GripInformation, bIsSecondaryGrip);
	TriggerComponent->DeactivateTrigger();
}

bool AWPNA_UnderbarrelWeapon::OnMagReleasePressed()
{
	if(IsGripped() && OnMagReleasePressedEvent.IsBound())
	{
		bMagReleasePressed = true;
		OnMagReleasePressedEvent.Broadcast();
		return true;
	}
	return false;
}

bool AWPNA_UnderbarrelWeapon::OnMagReleaseReleased()
{
	if(IsGripped() && IsMagReleasePressed() && OnMagReleaseReleasedEvent.IsBound())
	{
		bMagReleasePressed = false;
		OnMagReleaseReleasedEvent.Broadcast();
		return true;
	}
	bMagReleasePressed = false; // we must ensure that this is still set to false, in case no action is bound to release but to press.
	return false;
}

bool AWPNA_UnderbarrelWeapon::OnBoltReleasePressed()
{
	if(IsGripped() && OnBoltReleasePressedEvent.IsBound())
	{
		OnBoltReleasePressedEvent.Broadcast();
		return true;
	}
	return false;
}

bool AWPNA_UnderbarrelWeapon::OnBoltReleaseReleased()
{
	if(IsGripped() && OnBoltReleaseReleasedEvent.IsBound())
	{
		OnBoltReleaseReleasedEvent.Broadcast();
		return true;
	}
	return false;
}

bool AWPNA_UnderbarrelWeapon::IsGripped() const
{
	if(GetGunOwner())
	{
		if(const FBPActorGripInformation* GripInfo = GetGunOwner()->GetSecondaryGripInfo())
		{
			if(GripInfo->SecondaryGripInfo.SecondarySlotName == PrimaryGripName)
			{
				return true;
			}
		}
	}
	return false;
}

void AWPNA_UnderbarrelWeapon::OnStartFire_Implementation()
{
	if(IsGripped())
	{
		FiringComponent->StartFire();
	}
}

void AWPNA_UnderbarrelWeapon::OnStopFire_Implementation()
{
	FiringComponent->StopFire();
}

void AWPNA_UnderbarrelWeapon::OnFire_Implementation()
{
	if(GetGunOwner())
	{
		GetGunOwner()->AddCustomRecoil(
			FiringComponent->GetComponentTransform(),
			RecoilImpulse, AngularRecoilImpulse);
	}
}


