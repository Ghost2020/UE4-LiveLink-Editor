// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FaceGood_LiveLinkRuntime : ModuleRules
{
	public FaceGood_LiveLinkRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		CppStandard = CppStandardVersion.Cpp17;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
                //"FaceGoodPluginRuntime/Public",
            }
			);	
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
                //"FaceGoodPluginRuntime/Private",
            }
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
            }
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "Projects",
            }
            );
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
