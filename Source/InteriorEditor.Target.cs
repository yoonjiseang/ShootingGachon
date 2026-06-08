using UnrealBuildTool;
using System.Collections.Generic;

public class InteriorEditorTarget : TargetRules
{
	public InteriorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("Interior");
	}
}
