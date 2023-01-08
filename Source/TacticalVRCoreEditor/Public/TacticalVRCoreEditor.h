// This file is covered by the LICENSE file in the root of this plugin.

#pragma once
#include "Runtime/Core/Public/Modules/ModuleInterface.h"


class FTacticalVRCoreEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<class FComponentVisualizer> Visualizer);
	
	/** Array of component class names we have registered, so we know what to unregister afterwards */
	TArray<FName> RegisteredComponentClassNames;
	TArray<FName> RegisteredDetailCustomizers;
};

