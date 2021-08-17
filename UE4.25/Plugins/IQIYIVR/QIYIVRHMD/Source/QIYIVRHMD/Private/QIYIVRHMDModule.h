//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#pragma once

#include "IQIYIVRHMDModule.h"
#include "IHeadMountedDisplay.h"

#if WITH_EDITOR
#include "QIYUSettings.h"
#include "ISettingsModule.h"
#endif

#define LOCTEXT_NAMESPACE "QIYIVRHMD"

//-----------------------------------------------------------------------------
// FQIYIVRHMDModule Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class FQIYIVRHMDModule : public IQIYIVRHMDModule
{
    /** IHeadMountedDisplayModule implementation */
//    virtual TSharedPtr< class IHeadMountedDisplay, ESPMode::ThreadSafe > CreateHeadMountedDisplay() override;	
	virtual TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > CreateTrackingSystem() override;
	virtual TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe > GetVulkanExtensions() override;


	virtual void StartupModule() override
	{
		IHeadMountedDisplayModule::StartupModule();
#if WITH_EDITOR
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule != nullptr)
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "QIYU",
				LOCTEXT("QIYUSettingsName", "QIYU"),
				LOCTEXT("QIYUSettingsDescription", "Configure the QIYU plug-in."),
				GetMutableDefault<UQiyuSettings>()
			);
		}
#endif
	}

	static const FString QIYIVRHMDModuleString;

	/** Returns the key into the HMDPluginPriority section of the config file for this module */
	FString GetModuleKeyName() const
	{
		return QIYIVRHMDModuleString;
	}

	TSharedPtr< IHeadMountedDisplay, ESPMode::ThreadSafe > QIYIVRHMD;//maintain singleton instance, since for Android platforms, UEngine::PreExit() is not called, which means the destructor for FQIYIVRHMD is never called.  Nonetheless, CreateHeadMountedDisplay() can still be called an arbitrary number of times -- June 13, 2016
};

#undef LOCTEXT_NAMESPACE
