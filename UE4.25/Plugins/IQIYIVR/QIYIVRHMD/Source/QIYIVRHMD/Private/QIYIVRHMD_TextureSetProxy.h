//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#pragma once

#define USING_VULKAN_PATHWAY 1


#include "IQIYIVRHMDModule.h"
#include "QIYIVRHMDPrivateRHI.h"

class FQIYIVRTextureSet
{
public:
	FQIYIVRTextureSet();

	virtual ~FQIYIVRTextureSet();

	void SwitchToNextElement();

	FRHITexture* GetRenderTarget() const { return SwapChainImages[SwapChainImageIndex]; }

	inline int GetSwapChainImageIndex() const { return SwapChainImageIndex; }

protected:
	virtual void SetNativeResource(const FTextureRHIRef& RenderTargetTexture) = 0;

	TArray<FTextureRHIRef> SwapChainImages;
	int					   SwapChainImageIndex;
};

// FVulkanTexture2D needs the sample count, FOpenGLTexture2D does not
#if QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN
class FQIYIVRTextureSet_Vulkan* CreateTextureSetProxy_Vulkan(
	class FVulkanDynamicRHI* InVulkanRHI,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InArraySize,
	uint32 InNumSamples,
	uint8 InFormat,
	uint32 InFlags,
	uint32 InTargetableTextureFlags,
	FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture);
#endif // QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN

#if QIYIVR_HMD_SUPPORTED_PLATFORMS_OPENGLES
class FQIYIVRTextureSet_OpenGL* CreateTextureSetProxy_OpenGLES(
	class FOpenGLDynamicRHI* InGLRHI,
	GLenum InTarget,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InArraySize,
	uint8 InFormat,
	uint32 InFlags,
	uint32 InTargetableTextureFlags,
	FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture);
#endif // QIYIVR_HMD_SUPPORTED_PLATFORMS_OPENGLES