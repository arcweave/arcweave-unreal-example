// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ArcweaveDemoEditorTarget : TargetRules
{
	public ArcweaveDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ArcweaveDemo");
         #if UE_5_5_OR_LATER
             DefaultBuildSettings = BuildSettingsVersion.V5;
             CppStandard = CppStandardVersion.Cpp20;
         #endif
	}
}