// This file is covered by the LICENSE file in the root of this plugin.

#include "TacticalVRCore.h"

#define LOCTEXT_NAMESPACE "FTacticalVRCoreModule"

void FTacticalVRCoreModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FTacticalVRCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTacticalVRCoreModule, TacticalVRCore)