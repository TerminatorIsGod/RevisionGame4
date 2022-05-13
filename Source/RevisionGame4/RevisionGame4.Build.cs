// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RevisionGame4 : ModuleRules
{
	public RevisionGame4(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
