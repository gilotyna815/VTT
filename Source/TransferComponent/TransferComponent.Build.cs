using UnrealBuildTool;

public class TransferComponent: ModuleRules
{
    public TransferComponent(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
