//=============================================================================
//
//                 Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                              All Rights Reserved.
//
//=============================================================================

using UnrealBuildTool;

public class QIYIInput : ModuleRules
{
	public QIYIInput(ReadOnlyTargetRules Target) : base(Target)
	{

        string QIYISDKHeaderDirectory = System.IO.Path.Combine(ModuleDirectory, @"..\..\..\QIYIVRLib\Include");
        QIYISDKHeaderDirectory = System.IO.Path.GetFullPath(QIYISDKHeaderDirectory);

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                //"InputDevice",			// For IInputDevice.h
				//"HeadMountedDisplay",	// For IMotionController.h
				//"ImageWrapper",
                QIYISDKHeaderDirectory
				// ... add other private include paths required here ...
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
                    "ApplicationCore",
                    "Engine",
                    "InputCore",
                    "InputDevice",
                    "HeadMountedDisplay",
                    "QIYIVRHMD"
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
