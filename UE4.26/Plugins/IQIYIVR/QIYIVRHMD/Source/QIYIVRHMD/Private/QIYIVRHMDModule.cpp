//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMDModule.h"
#include "QIYIVRHMD.h"

const FString FQIYIVRHMDModule::QIYIVRHMDModuleString(TEXT("QIYIVR")); // an FString is a regular string array
const FName   IQIYIVRHMDModule::QIYIVRHMDModuleName(TEXT("QIYIVR")); // an FName is case insensitive

//-----------------------------------------------------------------------------
TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > FQIYIVRHMDModule::CreateTrackingSystem()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	TSharedRef< FQIYIVRHMD, ESPMode::ThreadSafe > HMD = FSceneViewExtensions::NewExtension<FQIYIVRHMD>();
	if (HMD->IsInitialized())
	{
		UE_LOG(LogSVR, Log, TEXT("QIYIVR->IsInitialized() == True"));

		return HMD;
	}

	UE_LOG(LogSVR, Log, TEXT("QIYIVR->IsInitialized() == Failed"));
#endif

	return nullptr;
}


//-----------------------------------------------------------------------------
TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe >  FQIYIVRHMDModule::GetVulkanExtensions()
{
//	if (!VulkanExtensions.IsValid())
//	{
//		VulkanExtensions = MakeShareable(new FQIYIVRHMDModule::FVulkanExtensions);
//	}
//
//	return VulkanExtensions;
	return nullptr;
}


IMPLEMENT_MODULE(FQIYIVRHMDModule, QIYIVRHMD)
