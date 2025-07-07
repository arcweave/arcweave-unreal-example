// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ArcweaveDemoTarget : TargetRules
{
	public ArcweaveDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ArcweaveDemo");
         #if UE_5_5_OR_LATER
             DefaultBuildSettings = BuildSettingsVersion.V5;
             CppStandard = CppStandardVersion.Cpp20;
         #endif
	}
}