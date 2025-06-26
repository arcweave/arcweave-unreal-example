using System;
using System.IO;
using UnrealBuildTool;
public class ArcscriptTranspiler : ModuleRules
{
    public ArcscriptTranspiler(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "antlr4-runtime.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "ArcscriptTranspiler.lib"));
            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add("antlr4-runtime.dll");
            PublicDelayLoadDLLs.Add("ArcscriptTranspiler.dll");
            // Ensure that the DLL is staged along with the executable
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/ArcscriptTranspiler/lib/antlr4-runtime.dll");
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/ArcscriptTranspiler/lib/ArcscriptTranspiler.dll");

            PublicIncludePaths.AddRange(new string[]
            {
                Path.Combine(ModuleDirectory, "Public")
            });

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "Projects",
                "CoreUObject",
                "Json",
                "JsonUtilities",
                "Engine",
                "DeveloperSettings"
            });
        }
    }
}
