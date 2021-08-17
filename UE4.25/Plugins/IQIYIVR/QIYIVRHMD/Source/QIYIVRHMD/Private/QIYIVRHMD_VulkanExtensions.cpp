//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_VulkanExtensions.h"

//-------------------------------------------------------------------------------------------------
// FVulkanExtensions
//-------------------------------------------------------------------------------------------------

#if 0
bool FVulkanExtensions::GetVulkanInstanceExtensionsRequired(TArray<const ANSICHAR*>& Out)
{
	TArray<VkExtensionProperties> InstanceProperties;
	{
		uint32_t PropertyCount;
		VulkanRHI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, nullptr);
		InstanceProperties.SetNum(PropertyCount);
		VulkanRHI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, InstanceProperties.GetData());
	}

	// TODO - should NOT be hardcoding the strings - fetch them from the vulkan header - but of course they are defined only for desktops - will change in later Vulkan SDKs?
	 auto InstanceExtensions = static_array_of<const ANSICHAR*>(
		 "vkGetPhysicalDeviceImageFormatProperties2KHR",
		 "vkGetMemoryFdKHR",
		 "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR",
		 "vkGetSemaphoreFdKHR");

	 // TODO - don't use the Strcmp loop, use the TArray.Contains() instead.
	int32 ExtensionsFound = 0;
	for (int32 ExtensionIndex = 0; ExtensionIndex < InstanceExtensions.size(); ExtensionIndex++)
	{
		for (int32 PropertyIndex = 0; PropertyIndex < InstanceProperties.Num(); PropertyIndex++)
		{
			const ANSICHAR* PropertyExtensionName = InstanceProperties[PropertyIndex].extensionName;

			if (!FCStringAnsi::Strcmp(PropertyExtensionName, InstanceExtensions[ExtensionIndex]))
			{
				Out.Add(InstanceExtensions[ExtensionIndex]);
				ExtensionsFound++;
				break;
			}
		}
	}

	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD -- VulkanExtensions::GetVulkanInstanceExtensionsRequired()"));


	return ExtensionsFound == InstanceExtensions.size();
}

//-----------------------------------------------------------------------------

bool FVulkanExtensions::GetVulkanDeviceExtensionsRequired(struct VkPhysicalDevice_T *pPhysicalDevice, TArray<const ANSICHAR*>& Out)
{
	TArray<VkExtensionProperties> DeviceProperties;
	{
		uint32_t PropertyCount;
		VulkanRHI::vkEnumerateDeviceExtensionProperties((VkPhysicalDevice) pPhysicalDevice, nullptr, &PropertyCount, nullptr);
		DeviceProperties.SetNum(PropertyCount);
		VulkanRHI::vkEnumerateDeviceExtensionProperties((VkPhysicalDevice) pPhysicalDevice, nullptr, &PropertyCount, DeviceProperties.GetData());
	}

	auto DeviceExtensions = static_array_of<const ANSICHAR*>();  // None yet

	int32 ExtensionsFound = 0;
	for (int32 ExtensionIndex = 0; ExtensionIndex < DeviceExtensions.size(); ExtensionIndex++)
	{
		for (int32 PropertyIndex = 0; PropertyIndex < DeviceProperties.Num(); PropertyIndex++)
		{
			const ANSICHAR* PropertyExtensionName = DeviceProperties[PropertyIndex].extensionName;

			if (!FCStringAnsi::Strcmp(PropertyExtensionName, DeviceExtensions[ExtensionIndex]))
			{
				Out.Add(DeviceExtensions[ExtensionIndex]);
				ExtensionsFound++;
				break;
			}
		}
	}

	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD -- VulkanExtensions::GetVulkanDeviceExtensionsRequired()"));

	return ExtensionsFound == DeviceExtensions.size();
}
#endif