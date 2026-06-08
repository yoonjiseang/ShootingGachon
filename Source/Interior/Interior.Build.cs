using UnrealBuildTool;

public class Interior : ModuleRules
{
	public Interior(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Niagara",
			"Slate",
			"SlateCore",
			"UMG"
		});
	}
}
