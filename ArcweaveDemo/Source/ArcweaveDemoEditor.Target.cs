// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

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
        bOverrideBuildEnvironment = true;
#endif

#if UE_5_7_OR_LATER
        DefaultBuildSettings = BuildSettingsVersion.V6;
        bOverrideBuildEnvironment = true;
#endif

    }
}