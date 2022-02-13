// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_PistolLight.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_PistolLight : public ATVRWeaponAttachment
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UTVRHoverInputVolume* HoverInputComponent;
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UAudioComponent* LaserToggleSound;
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class USpotLightComponent* SpotLight;

public:
	AWPNA_PistolLight(const FObjectInitializer& OI);

	virtual void BeginPlay() override;

	class UAudioComponent* GetToggleSound() const {return LaserToggleSound; }
	class UTVRHoverInputVolume* GetHoverInputComponent() const {return HoverInputComponent; }
	
	UFUNCTION()
	virtual void ToggleLight(class UGripMotionControllerComponent* UsingHand);
	
	UFUNCTION()
	virtual void ToggleLaser(class UGripMotionControllerComponent* UsingHand) {}

	virtual bool IsLightOn() const {return bIsLightOn;}

	protected:
	bool bIsLightOn;
};
