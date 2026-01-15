// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TeamSpaceProject : ModuleRules
{
	public TeamSpaceProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystem", 
			"OnlineSubsystemUtils", "OnlineSubsystemSteam" });

        PrivateDependencyModuleNames.AddRange(new string[] { });

    }

}
