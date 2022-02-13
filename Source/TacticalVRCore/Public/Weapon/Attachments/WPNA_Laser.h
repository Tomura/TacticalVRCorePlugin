// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_Laser.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_Laser : public ATVRWeaponAttachment
{
	GENERATED_BODY()

	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UTVRHoverInputVolume* HoverInputComponent;
 
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UStaticMeshComponent* LaserBeam;
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UStaticMeshComponent* LaserImpactMesh;
	
	UPROPERTY(Category = "Laser", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	class UAudioComponent* LaserToggleSound;
	
protected:
	bool bIsLaserOn;

	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	float Spread;
	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	float BaseThickness;

public:
	AWPNA_Laser(const FObjectInitializer& OI);

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void ToggleLaser(class UGripMotionControllerComponent* UsingHand);

	virtual bool IsLaserOn() const {return bIsLaserOn;}


};
