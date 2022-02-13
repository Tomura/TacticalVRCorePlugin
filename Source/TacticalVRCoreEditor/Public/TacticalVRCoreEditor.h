// This file is covered by the LICENSE file in the root of this plugin.

#pragma once
#include "Runtime/Core/Public/Modules/ModuleInterface.h"


class FTacticalVRCoreEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

