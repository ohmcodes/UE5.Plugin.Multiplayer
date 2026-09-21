// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MPPlugin58 : ModuleRules
{
	public MPPlugin58(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "OnlineSubsystemSteam",
            "OnlineSubsystem"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MPPlugin58",
			"MPPlugin58/Variant_Platforming",
			"MPPlugin58/Variant_Platforming/Animation",
			"MPPlugin58/Variant_Combat",
			"MPPlugin58/Variant_Combat/AI",
			"MPPlugin58/Variant_Combat/Animation",
			"MPPlugin58/Variant_Combat/Gameplay",
			"MPPlugin58/Variant_Combat/Interfaces",
			"MPPlugin58/Variant_Combat/UI",
			"MPPlugin58/Variant_SideScrolling",
			"MPPlugin58/Variant_SideScrolling/AI",
			"MPPlugin58/Variant_SideScrolling/Gameplay",
			"MPPlugin58/Variant_SideScrolling/Interfaces",
			"MPPlugin58/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
