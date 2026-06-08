using UnrealBuildTool;
using System.Collections.Generic;

public class InteriorTarget : TargetRules
{
	public InteriorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("Interior");
	}
}
