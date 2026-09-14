using UnrealBuildTool;

public class ArcweaveDemo : ModuleRules
{
    public ArcweaveDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "arcweave" });
        PrivateDependencyModuleNames.AddRange(new[] { "InputCore", "Slate", "SlateCore" });
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
