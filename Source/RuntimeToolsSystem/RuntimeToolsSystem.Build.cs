using UnrealBuildTool;

public class RuntimeToolsSystem: ModuleRules
{
    public RuntimeToolsSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "RenderCore",
            "InteractiveToolsFramework",
            "MeshDescription",
            "StaticMeshDescription",
            "GeometryCore",
            "DynamicMesh",
            "GeometryFramework",
            "MeshConversion",
            "ModelingComponents",
            "MeshModelingTools",
            "MeshModelingToolsExp",
            "RuntimeGeometryUtils",
            "TransferComponent"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
             "Slate",
             "SlateCore"
         });
    }
}
