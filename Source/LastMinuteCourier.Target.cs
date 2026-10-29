using UnrealBuildTool;
using System.Collections.Generic;

public class LastMinuteCourierTarget : TargetRules
{
    public LastMinuteCourierTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("LastMinuteCourier");
    }
}
