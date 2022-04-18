// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRGunAnimInstance.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/TVRMagazine.h"
#include "Weapon/Component/TVRChargingHandleInterface.h"
#include "Weapon/Component/TVRGunFireComponent.h"
#include "Weapon/Component/TVRMagWellComponent.h"
#include "Weapon/Component/TVRTriggerComponent.h"

UTVRGunAnimInstance::UTVRGunAnimInstance(const FObjectInitializer& OI) : Super(OI)
{
    Bolt = 0.f;
    ChargingHandle = 0.f;
    Trigger = 0.f;
    bBoltReleasePressed = false;
    bMagazineReleasePressed = false;

	FiringMode = ETVRFireMode::Single;
	SelectorValue = 0.f;
	bSelectorInitialized= false;
	ChargingHandleStroke = 10.f;
	bFirstRoundEjected = false;
	SelectorLerpSpeed = 300.f;
}

void UTVRGunAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    ATVRGunBase* Gun = GetGunOwner();
    if(Gun)
    {
    	if(const auto TriggerComp = Gun->GetTriggerComponent())
    	{
    		Trigger = TriggerComp->GetTriggerValue();
    	}
    	else
    	{
    		Trigger = 0.f;
    	}
    	if(const auto ChargingHandleComp = Gun->GetChargingHandleInterface())
    	{
    		ChargingHandleStroke = ITVRChargingHandleInterface::Execute_GetMaxTavel(Gun->GetChargingHandleInterface());
    		bIsChargingHandleGrabbed = ITVRChargingHandleInterface::Execute_IsInUse(Gun->GetChargingHandleInterface());        	
    		ChargingHandleGrabType = ITVRChargingHandleInterface::Execute_GetGrabLocation(Gun->GetChargingHandleInterface());
			ChargingHandle = ITVRChargingHandleInterface::Execute_GetProgress(Gun->GetChargingHandleInterface());
    		ChargingHandleDistance = ChargingHandle * ChargingHandleStroke;
    	}
    	
        Bolt = Gun->GetBoltProgress();
    	BoltDistance = Bolt * ChargingHandleStroke;
    	Hammer = Gun->GetHammerProgress();
        bBoltReleasePressed = Gun->IsBoltReleasePressed();
        bMagazineReleasePressed = Gun->IsMagReleasePressed();
        bIsBoltLocked = Gun->IsBoltLocked();
    	
    	FiringMode = Gun->GetFiringComponent()->GetCurrentFireMode();

    	if(bSelectorInitialized)
    	{
    		const float SelectorTarget = GetSelectorTargetValue();
    		SelectorValue = FMath::FInterpConstantTo(SelectorValue, SelectorTarget, DeltaSeconds, SelectorLerpSpeed);
    	}
    	else
    	{
    		SelectorValue = GetSelectorTargetValue();
    		bSelectorInitialized = true;
    	}

    	if(UTVRMagazineCompInterface* MagInterface = Gun->GetMagInterface())
    	{
    		MagInsertionProgress = MagInterface->GetAmmoInsertProgress();
    	}
    	else
    	{
    		MagInsertionProgress = 0.f;
    	}
    }
}

float UTVRGunAnimInstance::GetSelectorTargetValue() const
{
	switch(FiringMode)
	{
		case ETVRFireMode::Automatic:
			return SelectorValueAuto;
		case ETVRFireMode::Burst:
			return SelectorValueBurst;
		case ETVRFireMode::Single:
			return SelectorValueSingle;
	}
	return 0.f;
}


ATVRGunBase* UTVRGunAnimInstance::GetGunOwner() const
{
    AActor* OwningActor = GetOwningActor();
    return (OwningActor != nullptr) ? Cast<ATVRGunBase>(OwningActor) : nullptr;
}
