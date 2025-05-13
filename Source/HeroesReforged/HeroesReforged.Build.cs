// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeroesReforged : ModuleRules
{
	public HeroesReforged(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
