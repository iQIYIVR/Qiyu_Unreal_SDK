//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#pragma once

#include "Runtime/Launch/Resources/Version.h"

// create a simpler #def for engine versions
#if ENGINE_MAJOR_VERSION == 4
	#if  ENGINE_MINOR_VERSION == 19
		#define  UE4_419
	#elif  ENGINE_MINOR_VERSION == 20
		#define  UE4_420
	#elif  ENGINE_MINOR_VERSION == 21
		#define  UE4_421
	#elif  ENGINE_MINOR_VERSION == 22
		#define  UE4_422
	#elif  ENGINE_MINOR_VERSION == 23
		#define  UE4_423
	#endif
#endif

#ifdef UE4_419
	#include "ModuleManager.h"  // 4.19
#else
	#include "Modules/ModuleManager.h" // 4.20
#endif

#include "IHeadMountedDisplayModule.h"
#include "HeadMountedDisplayTypes.h"
#include "HeadMountedDisplayBase.h"
#include <array>

// std::array is a compile time array, thus it's fixed size. You just need to also say how many elements. Piffle...
// This lovely template code recursively calls itself to count then create a std::array of the correct size
// just do something like;
// auto arr = static_array_of<int>(1, 2, 3, 4, 5);
// auto arr2 = static_array_of<const char*>( "fred", "barney", "wilma", "pebbles");
//
// just some template magic to allow one to create a std::array 
// without counting the number of elements manually...
template <typename V, typename... T>
constexpr auto static_array_of(T&&... t)
->std::array < V, sizeof...(T) >
{
	return { { std::forward<T>(t)... } }; // yes double brackets are necessary
}

#define QIYIVR_UNREAL

#define QIYIVR_SUPPORTED_PLATFORMS (PLATFORM_ANDROID && PLATFORM_ANDROID_ARM64)

#if QIYIVR_SUPPORTED_PLATFORMS  
#define QIYIVR_HMD_SUPPORTED_PLATFORMS_OPENGLES		1
#define QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN		0
#else 
#define QIYIVR_HMD_SUPPORTED_PLATFORMS_OPENGLES		0
#define QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN		0
#endif // QIYIVR_SUPPORTED_PLATFORMS



/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class IQIYIVRHMDModule : public IHeadMountedDisplayModule
{

public:

	static const FName QIYIVRHMDModuleName;
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IQIYIVRHMDModule& Get()
	{
		return FModuleManager::LoadModuleChecked< IQIYIVRHMDModule >(QIYIVRHMDModuleName);
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(QIYIVRHMDModuleName);
	}
};
