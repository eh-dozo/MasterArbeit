// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MasterArbeit : ModuleRules
{
	public MasterArbeit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"NavigationSystem",
			"Niagara",
			"EnhancedInput",
			"TurnBasedCombatSystem",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"LlamaRunner",
			"Json",
			"JsonUtilities"
		]);

		PrivateDependencyModuleNames.AddRange([
			"DeveloperSettings"
		]);

		const bool bVerboseLogging = false;
		if (Target.Type == TargetType.Editor && bVerboseLogging)
			PublicDefinitions.Add("MA_IS_VERBOSE=1");
		else
			PublicDefinitions.Add("MA_IS_VERBOSE=0");
	}
}