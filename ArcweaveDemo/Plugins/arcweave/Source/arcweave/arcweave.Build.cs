// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class arcweave : ModuleRules
{
	public arcweave(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		// for windows only? check this afterwards?
		bUseRTTI = true;
		// we are using exceptions so we have to enable that
		bEnableExceptions = true;
		// enable this to 1 if we have memory issues
		PublicDefinitions.Add("ANTLR4_USE_THREAD_LOCAL_CACHE=0");
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"antlr4",
				"Projects",
				"Json",
				"JsonUtilities", 
				"Engine",
				"DeveloperSettings"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
