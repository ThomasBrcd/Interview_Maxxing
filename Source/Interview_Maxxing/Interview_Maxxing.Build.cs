// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Interview_Maxxing : ModuleRules
{
	public Interview_Maxxing(ReadOnlyTargetRules Target) : base(Target)
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
			"Json", 
			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Interview_Maxxing",
			"Interview_Maxxing/Variant_Horror",
			"Interview_Maxxing/Variant_Horror/UI",
			"Interview_Maxxing/Variant_Shooter",
			"Interview_Maxxing/Variant_Shooter/AI",
			"Interview_Maxxing/Variant_Shooter/UI",
			"Interview_Maxxing/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
