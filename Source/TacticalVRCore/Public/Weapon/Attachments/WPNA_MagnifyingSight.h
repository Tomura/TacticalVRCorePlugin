// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Attachments/WPNA_Sight.h"
#include "WPNA_MagnifyingSight.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TACTICALVRCORE_API AWPNA_MagnifyingSight : public AWPNA_Sight
{
	GENERATED_BODY()

	UPROPERTY(Category = "Magnifying Sight", EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	USceneCaptureComponent2D* SceneCaptureComponent;
	
public:
	AWPNA_MagnifyingSight(const FObjectInitializer& OI);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnOwnerGripped_Implementation(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo) override;
	virtual void OnOwnerDropped_Implementation(UGripMotionControllerComponent* GrippingHand, const FBPActorGripInformation& GripInfo, bool bSocketed) override;

	UFUNCTION(Category = "Sight", BlueprintCallable)
	virtual float GetZoomLevelFromCurve(float Time) const;
protected:

	/** This function needs to be overridden in Blueprint to set the material accordingly. */
	UFUNCTION(Category = "Sight", BlueprintImplementableEvent)
	void SetReticleMaterial(UMaterialInterface* NewMaterial);

	/** Radius of the aperture in uu. Used for FOV calculation. */
	UPROPERTY(Category= "Sight", EditDefaultsOnly)
	float ApertureRadius;

	/** Zoom Factor of the scope. 3 means 3 times magnification. Adjust BaseFactor1X first to setup a proper 1X level. */
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	float ZoomFactor;

	/** Basic Zoom factor. Used */
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	float BaseFactor1x;

	/** Base Material to use for the Reticle Material*/
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* ReticleMaterialTemplate;
	
	/** Material used when the weapon is not being held to prevent seeing render textures*/
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* ReticleMaterialPassive;
	
	/** The Instance of the Reticle Material. Use this to modify parameters. */
	UPROPERTY(Category= "Sight", BlueprintReadOnly)
	UMaterialInstanceDynamic* ReticleMaterial;

	/** Is this a first focal point reticle, i.E. reticle is zoomed with the image */
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	bool bFirstFocalPointReticle;
	
	/** Use simplified approach for adapting camera FOV to distance to eyes */
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	bool bUseSimpleApproach;

	/** Use simplified approach for adapting camera FOV to distance to eyes */
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	bool bIsVariableOptic;
	
	/** Curve used to correlate a float value to a zoom value. To be used with dials, levers, etc. */
	UPROPERTY(Category= "Sight", EditDefaultsOnly, BlueprintReadWrite)
	FRuntimeFloatCurve ZoomDialCurve;
};
