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
	
	UPROPERTY(Category = "Light", EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UTVRHoverInputVolume* HoverInputComponent;
	
	UPROPERTY(Category = "Light", EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UAudioComponent* LightToggleSound;
	
	UPROPERTY(Category = "Light", EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class USpotLightComponent* SpotLight;

public:
	AWPNA_Light(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	
	UFUNCTION(Category= "Light", BlueprintCallable)
	virtual void ToggleLight(class UGripMotionControllerComponent* UsingHand);

	UFUNCTION(Category= "Light", BlueprintCallable)
	virtual bool IsLightOn() const {return bIsLightOn;}

protected:
	bool bIsLightOn;
	
	UPROPERTY(Category = "Light", EditDefaultsOnly)
	FName LightOnMaterialParam;	
	UPROPERTY(Category = "Light", EditDefaultsOnly)
	uint8 LightMaterialSlot;

	UPROPERTY()
	UMaterialInstanceDynamic* LightMaterialInstance;
};
