// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"

#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "WPNA_Laser.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLaserToggleEvent, bool, bIsLaserOn, class UGripMotionControllerComponent*, UsingHand);

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

public:
	UPROPERTY(Category = "Laser", BlueprintAssignable)
	FLaserToggleEvent EventOnToggleLaser;
	
protected:
	bool bIsLaserOn;

	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	float Spread;
	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	float BaseThickness;

	
	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	FName LaserOnMaterialParam;	
	UPROPERTY(Category = "Laser", EditDefaultsOnly)
	uint8 LaserMaterialSlot;

	UPROPERTY()
	UMaterialInstanceDynamic* LaserMaterialInstance;

public:
	AWPNA_Laser(const FObjectInitializer& OI);

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(Category= "Laser", BlueprintCallable)
	virtual void ToggleLaser(class UGripMotionControllerComponent* UsingHand);
	
	UFUNCTION(Category= "Laser", BlueprintImplementableEvent)
	void OnToggleLaser(bool NewLaserStatus, class UGripMotionControllerComponent* UsingHand);

	UFUNCTION(Category= "Laser", BlueprintCallable)
	virtual bool IsLaserOn() const {return bIsLaserOn;}

	UFUNCTION(Category= "Laser", BlueprintImplementableEvent)
	void PostLaserTrace(const FHitResult& Hit);

};
