using UnrealBuildTool;
using System.Collections.Generic;

public class LastMinuteCourierEditorTarget : TargetRules
{
    public LastMinuteCourierEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("LastMinuteCourier");
    }
}
