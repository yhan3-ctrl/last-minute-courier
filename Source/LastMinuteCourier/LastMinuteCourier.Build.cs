using UnrealBuildTool;

public class LastMinuteCourier : ModuleRules
{
    public LastMinuteCourier(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Json" });
    }
}
