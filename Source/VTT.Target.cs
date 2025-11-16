// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class VTTTarget : TargetRules
{
	public VTTTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("VTT");
		ExtraModuleNames.Add("RuntimeToolsSystem");
		ExtraModuleNames.Add("TransferComponent");
    }
}
