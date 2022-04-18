// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TVRCoreGameplaySettings.generated.h"


UENUM(BlueprintType) 
enum class ELocomotionStyle : uint8 {
	LOC_ContiniousHMD	UMETA(DisplayName = "Continuous HMD"),
	LOC_ContiniousHand	UMETA(DisplayName = "Continuous Controller"),
};

UENUM(BlueprintType)
enum class ERotationStyle : uint8 {
	ROT_Snap			UMETA(DisplayName = "Snap Turning"),
	ROT_ContinuousSnap	UMETA(DisplayName = "Continous Snap Turning"),
	ROT_Continuous		UMETA(DisplayName = "Continous Turning"),
};

UENUM(BlueprintType)
enum class EStockType : uint8 {
	ST_None,
	ST_VirtualStock,
	ST_PhysicalStock
};


/**
 * 
 */
UCLASS(Config = Game)
class TACTICALVRCORE_API UTVRCoreGameplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

	UTVRCoreGameplaySettings(const FObjectInitializer& OI);

public:
	/** Currently used Locomotion Style */
	UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite, Config)
	ELocomotionStyle LocomotionStyle;

	/** Currently used Turning Style */
	UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite, Config)
	ERotationStyle TurnStyle;

	/** Turn speed in deg/s */
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
	float TurnSpeed;
	/** Turn Increment in deg*/
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
	float TurnIncrement;
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
    float SnapTurnSpeed;

	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
	FFloatRange SnapTurnHisteresis;
	
	
	class UMaterialParameterCollection* SightMaterialParameterCollection;
	
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
	FColor SightReticleColor;
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
	FColor PistolNightSightColor;
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
	FColor LaserColor;


	/** Camera Fade duration during snap turn in s*/
	UPROPERTY(Category = "Movement", BlueprintReadWrite, EditAnywhere, Config)
    float SnapTurnFadeDuration;

	
	/** Gun stock type */
	UPROPERTY(Category = "Guns", BlueprintReadWrite, EditAnywhere, Config)
	EStockType GunStockType;

	/** Strength of virtual gun stock support */
	UPROPERTY(Category = "Guns", BlueprintReadWrite, EditAnywhere, Config)
	float VirtualStockStrength;

	/** Distance of Secondary Grip Location from Primary Grip Location. X - Forward (Barrel), Y - Right, Z - Up */
	UPROPERTY(Category = "Guns", BlueprintReadWrite, EditAnywhere, Config)
	FVector PhysicalStockSecondaryOffset;

	UFUNCTION(Category = "Settings", BlueprintCallable, BlueprintPure, meta=(DisplayName="Get Tactical VR Core Gameplay Settings"))
	static UTVRCoreGameplaySettings* Get();

	/** Blueprint Node to expose the Save Config Function*/
	UFUNCTION(Category = "Settings", BlueprintCallable, meta=(DisplayName="Save Config"))
	void BPSaveConfig();

	/** Load the default config file and saves it */
	UFUNCTION(Category = "Settings", BlueprintCallable, meta=(DisplayName="Reset Config"))
	void ResetConfig();

	void ApplySettings();
};
