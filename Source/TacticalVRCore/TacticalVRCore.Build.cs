using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
    public class TacticalVRCore : ModuleRules
	{
		public TacticalVRCore(ReadOnlyTargetRules Target) : base(Target)
		{
			// PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			// not needed but I like to have a define that can be used to identify whether a plugin is actually used during build
			PublicDefinitions.Add("TACTICALVRCORE_PLUGIN=1");
		
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				});
				
		
			PrivateIncludePaths.AddRange(
				new string[] {
					// ... add other private include paths required here ...
				});
			
		
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",				
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore",
					"InputCore",
					"PhysicsCore",
					"UMG",
					"DeveloperSettings",
					"GameplayTags",
                    "XRBase",
                    "HeadMountedDisplay",
					"VRExpansionPlugin",
					"OpenXRExpansionPlugin"
				});
			
		
		
			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				});
		}
	}
}

