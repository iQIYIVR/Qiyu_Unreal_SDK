//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_TextureSetProxy.h"

#include "sxrApi.h"
#include "VulkanRHIPrivate.h"
#include "QIYIVRHMDPrivateRHI.h"
#include "QIYIVR_CVars.h"


#if QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN



struct FSVRVulkanSemaphore
{
	FSVRVulkanSemaphore(FVulkanDevice* InDevice)
		: Device(*InDevice)
		, SemaphoreHandle(VK_NULL_HANDLE)
	{
		VkSemaphoreCreateInfo PresentCompleteSemaphoreCreateInfo;
		FMemory::Memzero(PresentCompleteSemaphoreCreateInfo);

		PresentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		PresentCompleteSemaphoreCreateInfo.pNext = nullptr;
		PresentCompleteSemaphoreCreateInfo.flags = 0;

		VkExportSemaphoreCreateInfoKHR ExportSemaphoreCreateInfo;
		FMemory::Memzero(ExportSemaphoreCreateInfo);

		VkExternalSemaphoreHandleTypeFlagBitsKHR ExportType = VULKAN_SEMAPHORE_EXPORT_TYPE;

		ExportSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR;
		ExportSemaphoreCreateInfo.pNext = NULL;

		ExportSemaphoreCreateInfo.handleTypes = ExportType;

		// Make sure we support exporting a semaphore as an object of this type 
		VkPhysicalDeviceExternalSemaphoreInfoKHR ExternalSemaphoreCreateInfo;
		FMemory::Memzero(ExternalSemaphoreCreateInfo);
		ExternalSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR;
		ExternalSemaphoreCreateInfo.handleType = ExportType;

		VkExternalSemaphorePropertiesKHR ExternalSemaphoreProperties;
		FMemory::Memzero(ExternalSemaphoreProperties);

		ExternalSemaphoreProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR;

		vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(Device.GetPhysicalHandle(), &ExternalSemaphoreCreateInfo, &ExternalSemaphoreProperties);

		checkf(ExternalSemaphoreProperties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR,
			   TEXT("Semaphore cannot be exported as an object of this type!"));

		PresentCompleteSemaphoreCreateInfo.pNext = &ExportSemaphoreCreateInfo;

		// Create semaphore
		VERIFYVULKANRESULT(VulkanRHI::vkCreateSemaphore(Device.GetInstanceHandle(), &PresentCompleteSemaphoreCreateInfo, nullptr, &SemaphoreHandle));
	}

	~FSVRVulkanSemaphore()
	{
		check(SemaphoreHandle != VK_NULL_HANDLE);
		Device.GetDeferredDeletionQueue().EnqueueResource(VulkanRHI::FDeferredDeletionQueue::EType::Semaphore, SemaphoreHandle);
		SemaphoreHandle = VK_NULL_HANDLE;
	}

	// DARCHARD: bad name, but once you've got this FD this Semaphore is basically dead to Vulkan so we have to refresh the SemaphoreHandle
	int GetSemaphoreFd()
	{
		int Result = ~0;

		VkSemaphoreGetFdInfoKHR SemaphoreFdInfo;
		FMemory::Memzero(SemaphoreFdInfo);

		SemaphoreFdInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
		SemaphoreFdInfo.semaphore = SemaphoreHandle;
		SemaphoreFdInfo.handleType = VULKAN_SEMAPHORE_EXPORT_TYPE;

		VERIFYVULKANRESULT(vkGetSemaphoreFdKHR(Device.GetInstanceHandle(), &SemaphoreFdInfo, &Result));
		check(Result >= 0);

		return Result;
	}

	VkSemaphore GetHandle()	{ return SemaphoreHandle; }

private:

	FVulkanDevice& Device;
	VkSemaphore SemaphoreHandle;
};

class FQIYIVRTextureSet_Vulkan : public FVulkanTexture2D, public FQIYIVRTextureSet
{
public:
	FQIYIVRTextureSet_Vulkan(
		class FVulkanDynamicRHI* InVuklanRHI,
		uint32 InSizeX,
		uint32 InSizeY,
		uint32 InArraySize,
		uint32 InNumSamples,
		uint8 InFormat, // pix
		uint32 InFlags,
		uint32 InTargetableTextureFlags);

	virtual ~FQIYIVRTextureSet_Vulkan() override;

	struct FSVRVulkanSemaphore* GetSemaphore() const
	{
		return Semaphores[SwapChainImageIndex];
	}

	const FVulkanTextureView* GetImageView() const
	{
		return &TextureViews[SwapChainImageIndex];
	}

	int GetFileDescriptor() const
	{
		return FileDescriptors[SwapChainImageIndex];
	}

	uint32 GetAllocationSize() const
	{
		return AllocationSize;
	}

	uint32 GetBytesPerPixel() const
	{
		return BytesPerPixel;
	}

protected:
	virtual void SetNativeResource(const FTextureRHIRef& RenderTargetTexture) override;

private:
	FQIYIVRTextureSet_Vulkan(const FQIYIVRTextureSet_Vulkan &) = delete;
	FQIYIVRTextureSet_Vulkan(FQIYIVRTextureSet_Vulkan &&) = delete;
	FQIYIVRTextureSet_Vulkan &operator=(const FQIYIVRTextureSet_Vulkan &) = delete;

	TArray<struct FSVRVulkanSemaphore*> Semaphores;
	TArray<FVulkanTextureView>			TextureViews;
	TArray<int>							FileDescriptors;

	uint32 AllocationSize;
	uint32 BytesPerPixel;

	friend class FVulkanCustomPresentSVR;
};

FQIYIVRTextureSet_Vulkan::FQIYIVRTextureSet_Vulkan(
	class FVulkanDynamicRHI* InVulkanRHI,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InArraySize,
	uint32 InNumSamples,
	uint8 InFormat, // pix
	uint32 InFlags,
	uint32 InTargetableTextureFlags
) : FVulkanTexture2D(
	*InVulkanRHI->GetDevice(),
	(EPixelFormat)InFormat,
	InSizeX,
	InSizeY,
	1,
	InNumSamples,
	InFlags,
	FRHIResourceCreateInfo())
{
	FVulkanDevice* Device = InVulkanRHI->GetDevice();
	EPixelFormat eFormat = (EPixelFormat)InFormat; // unint8 to enum

	uint32 EffectiveSamples = InNumSamples;
	InNumSamples = 1;

	check(CVars::GNumSwapchainImages > 0);
	TextureViews.AddZeroed(CVars::GNumSwapchainImages);
	Semaphores.AddZeroed(CVars::GNumSwapchainImages);
	FileDescriptors.AddZeroed(CVars::GNumSwapchainImages);

	for (int i = 0; i < CVars::GNumSwapchainImages; ++i)
	{
		Semaphores[i] = new FSVRVulkanSemaphore(Device);

		const VkPhysicalDeviceProperties& DeviceProperties = Device->GetDeviceProperties();

		checkf(GPixelFormats[InFormat].Supported, TEXT("Format %d"), InFormat);

		VkImageCreateInfo ImageCreateInfo;
		FMemory::Memzero(ImageCreateInfo);
		ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

		check(InSizeX <= DeviceProperties.limits.maxImageDimension2D);
		check(InSizeY <= DeviceProperties.limits.maxImageDimension2D);
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // TODO?
		ImageCreateInfo.format = UEToVkTextureFormat(eFormat, false);

		checkf(ImageCreateInfo.format != VK_FORMAT_UNDEFINED, TEXT("Pixel Format %d not defined!"), InFormat);

		ImageCreateInfo.extent.width = InSizeX;
		ImageCreateInfo.extent.height = InSizeY;
		ImageCreateInfo.extent.depth = 1;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1; // TODO
		ImageCreateInfo.flags = 0;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

		ImageCreateInfo.usage = 0;
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		//@TODO: should everything be created with the source bit?
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

		if (InTargetableTextureFlags & TexCreate_Presentable)
		{
			ImageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		else if (InTargetableTextureFlags & (TexCreate_RenderTargetable | TexCreate_DepthStencilTargetable))
		{
			ImageCreateInfo.usage |= (InTargetableTextureFlags & TexCreate_RenderTargetable) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		}
		else if (InTargetableTextureFlags & (TexCreate_DepthStencilResolveTarget))
		{
			ImageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		}
		else if (InTargetableTextureFlags & TexCreate_ResolveTargetable)
		{
			ImageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		}

		if (InTargetableTextureFlags & TexCreate_UAV)
		{
			ImageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.queueFamilyIndexCount = 0;
		ImageCreateInfo.pQueueFamilyIndices = nullptr;

		switch (InNumSamples)
		{
			case 1:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				break;
			case 2:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_2_BIT;
				break;
			case 4:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
				break;
			case 8:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_8_BIT;
				break;
			case 16:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_16_BIT;
				break;
			case 32:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_32_BIT;
				break;
			case 64:
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_64_BIT;
				break;
			default:
				checkf(0, TEXT("Unsupported number of samples %d"), InNumSamples);
				break;
		}

		VkPhysicalDeviceExternalImageFormatInfoKHR DeviceExternalImageFormatInfo;
		FMemory::Memzero(DeviceExternalImageFormatInfo);
		DeviceExternalImageFormatInfo.sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR;
		DeviceExternalImageFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

		VkPhysicalDeviceImageFormatInfo2KHR DeviceImageFormatInfo;
		FMemory::Memzero(DeviceImageFormatInfo);

		DeviceImageFormatInfo.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
		DeviceImageFormatInfo.format = ImageCreateInfo.format;
		DeviceImageFormatInfo.type   = ImageCreateInfo.imageType;
		DeviceImageFormatInfo.tiling = ImageCreateInfo.tiling;
		DeviceImageFormatInfo.usage  = ImageCreateInfo.usage;
		DeviceImageFormatInfo.flags  = ImageCreateInfo.flags;
		DeviceImageFormatInfo.pNext  = &DeviceExternalImageFormatInfo;

		VkExternalImageFormatPropertiesKHR ExternalImageFormatProperties;
		FMemory::Memzero(ExternalImageFormatProperties);

		ExternalImageFormatProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR;

		VkImageFormatProperties2KHR ImageFormatProperties;
		FMemory::Memzero(ImageFormatProperties);

		ImageFormatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
		ImageFormatProperties.pNext = &ExternalImageFormatProperties;

		VERIFYVULKANRESULT(vkGetPhysicalDeviceImageFormatProperties2KHR(Device->GetPhysicalHandle(), &DeviceImageFormatInfo, &ImageFormatProperties));

		checkf(ExternalImageFormatProperties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR,
			TEXT("Image cannot be exported!"));

		checkf(ExternalImageFormatProperties.externalMemoryProperties.compatibleHandleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
			TEXT("Image cannot use an opaque FD!"));

		bool RequireDedicatedOnly = ((ExternalImageFormatProperties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_KHR) != 0);

		VkExternalMemoryImageCreateInfoKHR ExternalMemoryImageCreateInfo;
		FMemory::Memzero(ExternalMemoryImageCreateInfo);

		ExternalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR;
		ExternalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

		ImageCreateInfo.pNext = &ExternalMemoryImageCreateInfo;

		VkImage Image;
		VERIFYVULKANRESULT(VulkanRHI::vkCreateImage(Device->GetInstanceHandle(), &ImageCreateInfo, nullptr, &Image));

		// Fetch image size
		VkMemoryRequirements MemoryRequirements;
		VulkanRHI::vkGetImageMemoryRequirements(Device->GetInstanceHandle(), Image, &MemoryRequirements);

		uint32 MemoryTypeIndex = 0;
		VERIFYVULKANRESULT(Device->GetMemoryManager().GetMemoryTypeFromProperties(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &MemoryTypeIndex));

		// Allocate memory
		check(MemoryRequirements.size > 0);
		check(MemoryTypeIndex < Device->GetMemoryManager().GetMemoryProperties().memoryTypeCount);

		VkMemoryAllocateInfo Info;
		FMemory::Memzero(Info);
		Info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		Info.allocationSize = MemoryRequirements.size;
		Info.memoryTypeIndex = MemoryTypeIndex;

		VkExportMemoryAllocateInfoKHR ExportAllocationInfo = {};
		FMemory::Memzero(ExportAllocationInfo);

		VkMemoryDedicatedAllocateInfoKHR DedicatedAllocationInfo = {};
		FMemory::Memzero(DedicatedAllocationInfo);

		ExportAllocationInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
		ExportAllocationInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

		Info.pNext = &ExportAllocationInfo;

		if (RequireDedicatedOnly)
		{
			DedicatedAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
			DedicatedAllocationInfo.image = Image;

			DedicatedAllocationInfo.pNext = Info.pNext;
			Info.pNext = &DedicatedAllocationInfo;
		}

		VkDeviceMemory DeviceMemory;
		VERIFYVULKANRESULT(VulkanRHI::vkAllocateMemory(Device->GetInstanceHandle(), &Info, nullptr, &DeviceMemory));

		VERIFYVULKANRESULT(VulkanRHI::vkBindImageMemory(Device->GetInstanceHandle(), Image, DeviceMemory, 0));

		VkMemoryGetFdInfoKHR MemoryGetFdInfo;
		FMemory::Memzero(MemoryGetFdInfo);

		MemoryGetFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
		MemoryGetFdInfo.memory = DeviceMemory;
		MemoryGetFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

		int Fd;
		VERIFYVULKANRESULT(vkGetMemoryFdKHR(Device->GetInstanceHandle(), &MemoryGetFdInfo, &Fd));

		check(Fd >= 0);
		FileDescriptors[i] = Fd;

		uint32_t InNumMips = 1;
		InNumSamples = 1;  // should this be based on MSAA values?
		SwapChainImages[i] = InVulkanRHI->RHICreateTexture2DFromResource(eFormat, InSizeX, InSizeY, InNumMips, InNumSamples, Image, InTargetableTextureFlags).GetReference();
		//		SwapChainImages[i] = InVulkanRHI->RHICreateTexture2DFromVkImage(InFormat, InSizeX, InSizeY, Image, InTargetableTextureFlags);

		if (EffectiveSamples > 1)
		{
			FVulkanTextureBase* Texture = GetVulkanTextureFromRHITexture(SwapChainImages[i]);

			// The FRHIResourceCreateInfo parameter to the FVulkanSurface constructor is only used for the ClearValuebinding
			Texture->Surface = new FVulkanSurface(*Device, VK_IMAGE_VIEW_TYPE_2D, eFormat,
				/* dims x y z */ InSizeX, InSizeY, 1, /* array? */ false, /* array size */ 1, /* mips */ 1, /* sampls */ EffectiveSamples, InTargetableTextureFlags, FRHIResourceCreateInfo());
			Texture->MSAAView.Create(*Device, Texture->Surface->Image, VK_IMAGE_VIEW_TYPE_2D, Texture->Surface->GetFullAspectMask(), Texture->Surface->PixelFormat, Texture->Surface->ViewFormat, 0, 1, 0, 1);
		}

		if (i == 0)
		{
			AllocationSize = MemoryRequirements.size;
		}
		else
		{
			check(AllocationSize == MemoryRequirements.size);
		}

		if (EffectiveSamples > 1)
		{
			TextureViews[i] = GetVulkanTextureFromRHITexture(SwapChainImages[i])->MSAAView;
		}
		else
		{
			TextureViews[i].Create(*Device, Image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, eFormat, UEToVkTextureFormat(eFormat, false), 0, 1, 0, 1);
		}
	}

	BytesPerPixel = CalculateImageBytes(1, 1, 1, InFormat);

	SwapChainImageIndex = 0;
	SetNativeResource(SwapChainImages[SwapChainImageIndex]);
}


FQIYIVRTextureSet_Vulkan::~FQIYIVRTextureSet_Vulkan() {}

void FQIYIVRTextureSet_Vulkan::SetNativeResource(const FTextureRHIRef& RenderTargetTexture)
{
	FVulkanTextureBase* Texture = GetVulkanTextureFromRHITexture(RenderTargetTexture);

	Surface = Texture->Surface;
	DefaultView = Texture->DefaultView;

	if (GetNumSamples() > 1)
	{
		MSAASurface = Texture->MSAASurface;
		MSAAView = Texture->MSAAView;
	}
}

FQIYIVRTextureSet_Vulkan* CreateTextureSetProxy_Vulkan(
	class FVulkanDynamicRHI* InVulkanRHI,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InArraySize,
	uint32 InNumSamples,
	uint8 InFormat,
	uint32 InFlags,
	uint32 InTargetableTextureFlags,
	FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture)
{
	FQIYIVRTextureSet_Vulkan* TextureSetProxy = new FQIYIVRTextureSet_Vulkan(
		InVulkanRHI,
		InSizeX,
		InSizeY,
		InArraySize,
		InNumSamples,
		InFormat, // pix
		InFlags,
		InTargetableTextureFlags);

	OutTargetableTexture = OutShaderResourceTexture = TextureSetProxy;
	return TextureSetProxy;
}

void TextureSetProxy_SetupRenderLayer(const FQIYIVRTextureSet_Vulkan* TextureSetProxy, sxrRenderLayer* RenderLayer, const uint32* pFd)
{
	// If this is the second eye and we already exported the semaphore to a fd, don't do it again
	RenderLayer->vulkanInfo.renderSemaphore = pFd ? *pFd : TextureSetProxy->GetSemaphore()->GetSemaphoreFd();
	RenderLayer->imageType					= kTypeVulkan;
	RenderLayer->imageHandle				= TextureSetProxy->GetFileDescriptor();
	RenderLayer->vulkanInfo.memSize			= TextureSetProxy->GetAllocationSize();
	RenderLayer->vulkanInfo.width			= TextureSetProxy->GetSizeX();
	RenderLayer->vulkanInfo.height			= TextureSetProxy->GetSizeY();
	RenderLayer->vulkanInfo.numMips			= 1;
	RenderLayer->vulkanInfo.bytesPerPixel	= TextureSetProxy->GetBytesPerPixel();
}

void TextureSetProxy_SwitchToNextElement(FQIYIVRTextureSet_Vulkan* TextureSetProxy)
{
	TextureSetProxy->SwitchToNextElement();
}

VkSemaphore TextureSetProxy_GetSignalSemaphore(const FQIYIVRTextureSet_Vulkan* TextureSetProxy)
{
	return TextureSetProxy->GetSemaphore()->GetHandle();
}

VkImage TextureSetProxy_GetBackBufferImage(const FQIYIVRTextureSet_Vulkan* TextureSetProxy)
{
	FRHITexture* BackBuffer = TextureSetProxy->GetRenderTarget();
	FVulkanTextureBase* Texture = GetVulkanTextureFromRHITexture(BackBuffer);

	return (TextureSetProxy->GetNumSamples() > 1) ? Texture->MSAASurface->Image : Texture->Surface.Image;
}

VkImageView TextureSetProxy_GetBackBufferImageView(const FQIYIVRTextureSet_Vulkan* TextureSetProxy)
{
	// When MSAA is enabled, we store the MSAA image view in TextureSetProxy::TextureViews[]
	const FVulkanTextureView* TextureView = TextureSetProxy->GetImageView();
	return TextureView->View;
}
#endif 