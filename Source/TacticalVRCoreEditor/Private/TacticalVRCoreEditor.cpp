﻿// This file is covered by the LICENSE file in the root of this plugin.

#include "TacticalVRCoreEditor.h"
#include "TVRAttachPointDetails.h"
#include "TVREjectionPortVisualizer.h"
#include "TVRGunDetails.h"
#include "TVRMagazineWellVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "ComponentVisualizers/Public/ComponentVisualizers.h"

IMPLEMENT_MODULE(FTacticalVRCoreEditorModule, TacticalVRCoreEditor);

void FTacticalVRCoreEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Sight",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointSightDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Underbarrel",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointUnderbarrelDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Laser",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointLaserDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Light",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointLightDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_PistolLight",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointPistolLightDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Muzzle",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointMuzzleDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Stock",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointStockDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		"TVRAttachPoint_Barrel",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRAttachPointBarrelDetails::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		"TVRGunBase",
		FOnGetDetailCustomizationInstance::CreateStatic(&FTVRGunDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	RegisterComponentVisualizer(UTVRMagazineWell::StaticClass()->GetFName(), FTVRMagazineWellVisualizer::MakeInstance());
	RegisterComponentVisualizer(UTVREjectionPort::StaticClass()->GetFName(), FTVREjectionPortVisualizer::MakeInstance());
	
}

void FTacticalVRCoreEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Sight");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Underbarrel");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Laser");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Light");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_PistolLight");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Muzzle");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Stock");
	PropertyModule.UnregisterCustomClassLayout("TVRAttachPoint_Barrel");
	PropertyModule.UnregisterCustomClassLayout("TVRGunBase");
	
	if (GUnrealEd != nullptr)
	{
		for(const auto TestName : RegisteredComponentClassNames)
		{
			GUnrealEd->UnregisterComponentVisualizer(TestName);
		}
	}
}

void FTacticalVRCoreEditorModule::RegisterComponentVisualizer(FName ComponentClassName,
	TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}

	RegisteredComponentClassNames.Add(ComponentClassName);

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}
