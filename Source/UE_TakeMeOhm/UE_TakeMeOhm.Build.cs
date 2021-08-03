// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_TakeMeOhm : ModuleRules
{
	public UE_TakeMeOhm(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
