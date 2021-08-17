//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class QIYIVRHMD : ModuleRules
	{
        public QIYIVRHMD(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            bEnableUndefinedIdentifierWarnings = false;

            string  QIYISDKHeaderDirectory = System.IO.Path.Combine(ModuleDirectory, @"..\..\..\QIYIVRLib\Include");
			QIYISDKHeaderDirectory = System.IO.Path.GetFullPath(QIYISDKHeaderDirectory);

			string QIYISXRApiLibDirectory = System.IO.Path.Combine(QIYISDKHeaderDirectory, @"..\Libs\Android");
			QIYISXRApiLibDirectory = System.IO.Path.GetFullPath(QIYISXRApiLibDirectory);

			string APLPath = System.IO.Path.Combine(QIYISDKHeaderDirectory, @"..\..\QIYIVRHMD\Source\QIYIVRHMD");
			APLPath = System.IO.Path.GetFullPath(APLPath);

			string EngineRuntimePath = Path.GetFullPath(Target.RelativeEnginePath + @"\Source\Runtime");



			if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
			{
				PrivateIncludePaths.Add(EngineRuntimePath + @"\VulkanRHI\Private\Windows");
			}
			else
			{
				PrivateIncludePaths.Add(EngineRuntimePath + @"\VulkanRHI\Private\" + Target.Platform);
			}

			//System.Console.WriteLine(">>>>>>>>>>>>>>>>>>>>>>>>>>QIYISDKHeaderDirectory->" + QIYISDKHeaderDirectory);
			// Loop through List with foreach.

			PrivateIncludePaths.AddRange(
				new string[]{
                    "QIYIVRHMD/Private",
					EngineRuntimePath + @"\Renderer\Private",
					EngineRuntimePath + @"\Core\Private",
					EngineRuntimePath + @"\OpenGLDrv\Private",
					EngineRuntimePath + @"\VulkanRHI\Private",
					QIYISDKHeaderDirectory
				// ... add other private include paths required here ...
				}
			);

        //    PublicIncludePaths.AddRange(
        //    new string[]{
        //            EngineRuntimePath + @"\Core\Public",
        //    }
        //);

            string pathtoOpenGL = System.IO.Path.GetFullPath(Target.UEThirdPartySourceDirectory + "OpenGL");
			PrivateIncludePaths.Add(pathtoOpenGL);

			// 4.20/21/22 move Vulkan headers around a bit
			// best to get latest one. The oculus cs file figures out the version from the header source
			// for now assume one of two places for the header. Note that all versions are the same (for same API level)

			string pathtoVulkan = "";
			// pre 420 header location
			string pathtoVulkan419 = System.IO.Path.GetFullPath(Target.UEThirdPartySourceDirectory + @"Vulkan\Windows\Include\vulkan");
			// This is for 4.20
			string pathtoVulkan420 = System.IO.Path.GetFullPath(Target.UEThirdPartySourceDirectory + @"Vulkan\include\vulkan");


			if (File.Exists(pathtoVulkan419 + "/vulkan.h"))
				{
					pathtoVulkan = pathtoVulkan419;
				}
			else if (File.Exists(pathtoVulkan420 + "/vulkan.h"))
				{
					pathtoVulkan = pathtoVulkan420;
				}
			else
				{
				System.Console.WriteLine("  >>>>>>>>>>>>  QIYIHMD::PrivateIncludePaths -> cant find Vulkan.h");
				}

			// future TODO - might look for NDK or vulkan SDK
			// NOTE:  for some reason using relative path fails. but full path works
			PrivateIncludePaths.Add(System.IO.Path.GetFullPath(pathtoVulkan));

// 			string VulkanSDKPath = Environment.GetEnvironmentVariable("VULKAN_SDK");
// 
// 			bool bHaveVulkan = false;
// 			if (Target.Platform == UnrealTargetPlatform.Android)
// 			{
// 				// Note: header is the same for all architectures so just use arch-arm
// 				string NDKPath = Environment.GetEnvironmentVariable("NDKROOT");
// 				string NDKVulkanIncludePath = NDKPath + "/platforms/android-24/arch-arm/usr/include/vulkan";
// 
// 				// Use NDK Vulkan header if discovered, or VulkanSDK if available
// 				if (File.Exists(NDKVulkanIncludePath + "/vulkan.h"))
// 				{
// 					bHaveVulkan = true;
// 					PrivateIncludePaths.Add(NDKVulkanIncludePath);
// 				}
// 				else
// 				if (!String.IsNullOrEmpty(VulkanSDKPath))
// 				{
// 					// If the user has an installed SDK, use that instead
// 					bHaveVulkan = true;
// 					PrivateIncludePaths.Add(VulkanSDKPath + "/Include/vulkan");
// 				}
// 				else
// 				{
// 					// Fall back to the Windows Vulkan SDK (the headers are the same)
// 					bHaveVulkan = true;
// 					PrivateIncludePaths.Add(Target.UEThirdPartySourceDirectory + "Vulkan/Windows/Include/vulkan");
// 				}
// 			}
// 			
		
			foreach(string daPath in PrivateIncludePaths)
			{
				System.Console.WriteLine("  >>>>>>>>>>>>  QIYIHMD::PrivateIncludePaths -> " + daPath);
			}
			
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"RHI",
					"RenderCore",
					"Renderer",
					"HeadMountedDisplay",
                    "Slate",
					"SlateCore",
					"OpenGLDrv", 
					"VulkanRHI",
					"Json"
				}
				);

            PublicIncludePathModuleNames.Add("Launch");
            if (Target.Platform == UnrealTargetPlatform.Android)
            {
                PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
            }

            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");
			PrivateDependencyModuleNames.Add("OpenGLDrv");

			//AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
			//PrivateDependencyModuleNames.Add("VulkanRHI");


			// deleted "svrApi" module and removed references to it - we just link in the Android library directly

			if (Target.Platform == UnrealTargetPlatform.Android)
			{
                System.Console.WriteLine(@"Marshall Load Android =" + QIYISXRApiLibDirectory);

                AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(APLPath, "QIYIVRHMD_APL.xml"));

                //PublicAdditionalLibraries.Add(QIYISXRApiLibDirectory + "/armeabi-v7a/libsxrapi.so");
                PublicAdditionalLibraries.Add(QIYISXRApiLibDirectory + "/arm64-v8a/libsxrapi.so");
                PublicAdditionalLibraries.Add(QIYISXRApiLibDirectory + "/arm64-v8a/libashreader.so");
                PublicAdditionalLibraries.Add(QIYISXRApiLibDirectory + "/arm64-v8a/libqiyivrsdkcore.so");
                // toolchain will filter properly
                //PublicLibraryPaths.Add(QIYISXRApiLibDirectory + "/armeabi-v7a");
                PublicLibraryPaths.Add(QIYISXRApiLibDirectory + "/arm64-v8a");
                //PublicAdditionalLibraries.Add("libsxrapi.so");
				
			}

        }
	}
}
