//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_CustomPresent.h"
#include "QIYIVRHMD_TextureSetProxy.h"

#if QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN

#include "VulkanRHIPrivate.h"

PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR = nullptr;
PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR = nullptr;
PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = nullptr;
PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR = nullptr;

class FVulkanCustomPresentSVR : public FQIYIVRHMDCustomPresent//, public FVulkanCustomPresent
{
public:
	FVulkanCustomPresentSVR(FQIYIVRHMD* pHMD)
		: FQIYIVRHMDCustomPresent(pHMD)
		, mTextureSet(nullptr) {}
	virtual ~FVulkanCustomPresentSVR()
	{
		// DARCHARD: TODO - fixme
		//delete mTextureSet;
	}

	void SubmitFrame(const FPoseStateFrame& PoseState) override;

	virtual bool AllocateRenderTargetTexture(
		uint32 SizeX,
		uint32 SizeY,
		uint8 Format,
		uint32 NumMips,
		uint32 Flags,
		uint32 TargetableTextureFlags,
		FTexture2DRHIRef& OutTargetableTexture,
		FTexture2DRHIRef& OutShaderResourceTexture,
		uint32 NumSamples) override;

// 	virtual void* GetCustomPresentBaseRHI() override final
// 	{
// 		FVulkanCustomPresent* Base = static_cast<FVulkanCustomPresent*>(this);
// 		return Base;
// 	}
 
// 	virtual VkSemaphore GetSignalSemaphore() override;
// 
// 	virtual VkImage GetBackBufferImage() override;
// 	virtual VkImageView GetBackBufferImageView() override;

private:
	class FQIYIVRTextureSet_Vulkan* mTextureSet;
};

FQIYIVRHMDCustomPresent* CreateCustomPresent_Vulkan(class FQIYIVRHMD* pHMD)
{
	return new FVulkanCustomPresentSVR(pHMD);
}

bool FVulkanCustomPresentSVR::AllocateRenderTargetTexture(
	uint32 SizeX,
	uint32 SizeY,
	uint8 Format,
    uint32 NumMips, 
	uint32 Flags,
	uint32 TargetableTextureFlags,
	FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture,
	uint32 NumSamples)
{
	FVulkanDynamicRHI* VulkanRHI = static_cast<FVulkanDynamicRHI*>(GDynamicRHI);

	vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)VulkanDynamicAPI::vkGetInstanceProcAddr(VulkanRHI->GetInstance(), "vkGetPhysicalDeviceImageFormatProperties2KHR");
	vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)VulkanDynamicAPI::vkGetInstanceProcAddr(VulkanRHI->GetInstance(), "vkGetMemoryFdKHR");
	vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)VulkanDynamicAPI::vkGetInstanceProcAddr(VulkanRHI->GetInstance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
	vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)VulkanDynamicAPI::vkGetInstanceProcAddr(VulkanRHI->GetInstance(), "vkGetSemaphoreFdKHR");

	check(vkGetPhysicalDeviceImageFormatProperties2KHR && vkGetMemoryFdKHR && vkGetPhysicalDeviceExternalSemaphorePropertiesKHR && vkGetSemaphoreFdKHR);

 	static const auto MobileMSAACVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.MobileMSAA"));
 	static const int32 MobileMSAAValue = MobileMSAACVar ? MobileMSAACVar->GetValueOnRenderThread() : 1;

	static const auto MobileMultiViewCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
	static const bool bIsUsingMobileMultiView = GSupportsMobileMultiView && MobileMultiViewCVar && MobileMultiViewCVar->GetValueOnAnyThread() != 0;

	checkf(!bIsUsingMobileMultiView, TEXT("VK multiview is not supported yet"));

	static const auto MobileMultiViewDirectCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView.Direct"));
	static const bool bIsUsingMobileMultiViewDirect = bIsUsingMobileMultiView && MobileMultiViewDirectCVar && MobileMultiViewDirectCVar->GetValueOnAnyThread() != 0;

	uint32 Samples = 1;
	uint32 Layers = 1;
	if (bIsUsingMobileMultiViewDirect)
	{
		// TODO
		Layers = 2;
		bUsesArrayTexture = true;
	}
 	else if (!bIsUsingMobileMultiView && MobileMSAAValue > 1)
 	{
 		Samples = MobileMSAAValue;
 	}

	uint32 Width = SizeX / Layers;

	mTextureSet = CreateTextureSetProxy_Vulkan(
		VulkanRHI,
		Width,
		SizeY,
		Layers,
		Samples,
		Format,
		Flags,
		TargetableTextureFlags,
		OutTargetableTexture,
		OutShaderResourceTexture);

	return true;
 }

extern void TextureSetProxy_SetupRenderLayer(const FQIYIVRTextureSet_Vulkan* TextureSetProxy, sxrRenderLayer* RenderLayer, const uint32* pFd);
extern void TextureSetProxy_SwitchToNextElement(FQIYIVRTextureSet_Vulkan* TextureSetProxy);

void FVulkanCustomPresentSVR::SubmitFrame(const FPoseStateFrame& PoseState)
{
	sxrFrameParams FrameParams;
	memset(&FrameParams, 0, sizeof(FrameParams));
	FrameParams.frameIndex = PoseState.FrameNumber;

	TextureSetProxy_SetupRenderLayer(mTextureSet, &FrameParams.renderLayers[0], nullptr);
	CreateLayout(0.0f, 0.0f, 1.0f, 1.0f, &FrameParams.renderLayers[0].imageCoords);
	FrameParams.renderLayers[0].imageCoords.LowerUVs[2] = 0.5f;
	FrameParams.renderLayers[0].imageCoords.UpperUVs[2] = 0.5f;
	FrameParams.renderLayers[0].eyeMask = kEyeMaskLeft;
	FrameParams.renderLayers[0].layerFlags |= kLayerFlagOpaque;

	FrameParams.renderLayers[0].imageCoords.LowerUVs[1] = 1.f; // 0.f
	FrameParams.renderLayers[0].imageCoords.LowerUVs[3] = 1.f; // 0.f
	FrameParams.renderLayers[0].imageCoords.UpperUVs[1] = 0.f; // 1.f
	FrameParams.renderLayers[0].imageCoords.UpperUVs[3] = 0.f; // 1.f

	TextureSetProxy_SetupRenderLayer(mTextureSet, &FrameParams.renderLayers[1], &FrameParams.renderLayers[0].vulkanInfo.renderSemaphore);
	CreateLayout(0.0f, 0.0f, 1.0f, 1.0f, &FrameParams.renderLayers[1].imageCoords);
	FrameParams.renderLayers[1].imageCoords.LowerUVs[0] = 0.5f;
	FrameParams.renderLayers[1].imageCoords.UpperUVs[0] = 0.5f;
	FrameParams.renderLayers[1].eyeMask = kEyeMaskRight;
	FrameParams.renderLayers[1].layerFlags |= kLayerFlagOpaque;

	FrameParams.renderLayers[1].imageCoords.LowerUVs[1] = 1.f; // 0.f
	FrameParams.renderLayers[1].imageCoords.LowerUVs[3] = 1.f; // 0.f
	FrameParams.renderLayers[1].imageCoords.UpperUVs[1] = 0.f; // 1.f
	FrameParams.renderLayers[1].imageCoords.UpperUVs[3] = 0.f; // 1.f

	FrameParams.headPoseState = PoseState.Pose;
	FrameParams.minVsyncs = 1;

	sxrSubmitFrame(&FrameParams);
	TextureSetProxy_SwitchToNextElement(mTextureSet);
}

extern VkSemaphore TextureSetProxy_GetSignalSemaphore(const FQIYIVRTextureSet_Vulkan* TextureSetProxy);

// VkSemaphore FVulkanCustomPresentSVR::GetSignalSemaphore() 
// {
// 	return TextureSetProxy_GetSignalSemaphore(mTextureSet);
// }
// 
// extern VkImage TextureSetProxy_GetBackBufferImage(const FQIYIVRTextureSet_Vulkan* TextureSetProxy);
// 
// VkImage FVulkanCustomPresentSVR::GetBackBufferImage() 
// {
// 	return TextureSetProxy_GetBackBufferImage(mTextureSet);
// }
// 
// extern VkImageView TextureSetProxy_GetBackBufferImageView(const FQIYIVRTextureSet_Vulkan* TextureSetProxy);
// 
// VkImageView FVulkanCustomPresentSVR::GetBackBufferImageView() 
// {
// 	return TextureSetProxy_GetBackBufferImageView(mTextureSet);
// }

#endif //QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN