// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class antlr4 : ModuleRules
{
	public antlr4(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "antlr4-runtime.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("antlr4-runtime.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/antlr4/Win64/antlr4-runtime.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            /*PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/arcweaveLibrary/Mac/Release/libExampleLibrary.dylib");*/
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			/*string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "arcweaveLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);*/
		}
	}
}
