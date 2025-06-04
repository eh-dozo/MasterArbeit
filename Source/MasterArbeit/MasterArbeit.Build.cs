// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MasterArbeit : ModuleRules
{
	public MasterArbeit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
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
			"GameplayStateTreeModule"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			
		});
	}
}