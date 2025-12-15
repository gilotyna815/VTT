// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VTT : ModuleRules
{
	public VTT(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhancedInput",
	        "GeometryFramework",
            "InteractiveToolsFramework",
            "RuntimeGeometryUtils",
            "RuntimeToolsSystem",
            "MeshModelingTools",
            "MeshModelingToolsExp",
            "TransferComponent"
        });
    }
}
