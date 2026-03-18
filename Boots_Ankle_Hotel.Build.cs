// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Boots_Ankle_Hotel : ModuleRules
{
	public Boots_Ankle_Hotel(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore",
        "EnhancedInput",
        "Json",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
        "Slate",
        "SlateCore",
        "ApplicationCore",
        "NavigationSystem",
        "AIModule",
        "LevelSequence",
        "MovieScene",
        });

       // PublicIncludePaths.AddRange(new string[] {"Boots_Ankle_Hotel/Public"});
       // PrivateIncludePaths.AddRange(new string[] { "Boots_Ankle_Hotel/Private" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
