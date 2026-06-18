using UnrealBuildTool;

public class Fortress : ModuleRules
{
    public Fortress(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils", "AIModule", "NavigationSystem", "EnhancedInput", "UMG", "Slate", "SlateCore"});

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}