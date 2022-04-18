// This file is covered by the LICENSE file in the root of this plugin.

#include "Settings/TVRCoreGameplaySettings.h"
#include "Materials/MaterialParameterCollection.h"
#include "VRExpansionFunctionLibrary.h"

UTVRCoreGameplaySettings::UTVRCoreGameplaySettings(const FObjectInitializer& OI) : Super(OI)
{
	LocomotionStyle = ELocomotionStyle::LOC_ContiniousHMD;
	TurnStyle = ERotationStyle::ROT_Snap;
	TurnSpeed = 90.f;
	TurnIncrement = 30.f;
	SnapTurnFadeDuration = 0.2f;
	SnapTurnSpeed = 360.f;

	GunStockType = EStockType::ST_None;
	VirtualStockStrength = 0.f;
	PhysicalStockSecondaryOffset = FVector(35.f, 0.f, 0.f);
	 
	SightReticleColor = FColor(255, 0, 0);
	PistolNightSightColor = FColor(0, 255, 0);
	LaserColor = FColor(0, 255, 30);

	SnapTurnHisteresis = FFloatRange(0.3f, 0.65f);

	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> SightMPCObj(TEXT("/TacticalVRCore/MaterialParameterCollections/MPC_Sight.MPC_Sight"));
	if(SightMPCObj.Succeeded())
	{
		UE_LOG(LogTemp, Log, TEXT("Found SightMPC")); 
		SightMaterialParameterCollection = SightMPCObj.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTVRGameplaySettings could not find SightMPC")); 
	}
	SightMaterialParameterCollection = nullptr;
}

UTVRCoreGameplaySettings* UTVRCoreGameplaySettings::Get()
{
	return GetMutableDefault<UTVRCoreGameplaySettings>();
}

void UTVRCoreGameplaySettings::BPSaveConfig()
{
	SaveConfig();
}

void UTVRCoreGameplaySettings::ResetConfig()
{
	ReloadConfig(GetClass(), *(GetDefaultConfigFilename()));
	SaveConfig();
}

void UTVRCoreGameplaySettings::ApplySettings()
{
	if(SightMaterialParameterCollection)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(SightMaterialParameterCollection);
		// MPCInstance->SetVectorParameterValue(FName())
	}
}
