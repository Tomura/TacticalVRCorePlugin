// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_Light.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_Light : public ATVRWeaponAttachment
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Laser", EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UTVRHoverInputVolume* HoverInputComponent;
	
	UPROPERTY(Category = "Laser", EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UAudioComponent* LaserToggleSound;
	
	UPROPERTY(Category = "Laser", EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class USpotLightComponent* SpotLight;

public:
	AWPNA_Light(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void ToggleLight(class UGripMotionControllerComponent* UsingHand);

	virtual bool IsLightOn() const {return bIsLightOn;}

protected:
	bool bIsLightOn;
};
