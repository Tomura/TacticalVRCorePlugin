using UnrealBuildTool;

public class TacticalVRCore : ModuleRules
{
	public TacticalVRCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"HeadMountedDisplay",
				"VRExpansionPlugin",
				"OpenXRExpansionPlugin",
				"ForceTubeVRForUE4"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			});
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			});
	}
}
