// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TFG : ModuleRules
{
	public TFG(TargetInfo Target)
	{
        // PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
