//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#pragma once

//-------------------------------------------------------------------------------------------------
// OpenGLES
//-------------------------------------------------------------------------------------------------

#if QIYIVR_HMD_SUPPORTED_PLATFORMS_OPENGLES
#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"

// GL_QCOM_texture_foveated
#ifndef GL_QCOM_texture_foveated
#define GL_QCOM_texture_foveated 1
typedef void (GL_APIENTRYP PFNGLTEXTUREFOVEATIONPARAMETERSQCOMPROC) (GLuint texture, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea);
extern PFNGLTEXTUREFOVEATIONPARAMETERSQCOMPROC glTextureFoveationParametersQCOM;

#define GL_FOVEATION_ENABLE_BIT_QCOM                    0x0001
#define GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM         0x0002
#define GL_TEXTURE_PREVIOUS_SOURCE_TEXTURE_QCOM         0x8BE8
#define GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM           0x8BFB
#define GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM      0x8BFC
#define GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM          0x8BFD
#define GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM 0x8BFE
#define GL_FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM        0x8BFF
#endif // GL_QCOM_texture_foveated

#endif // QIYIVR_HMD_SUPPORTED_PLATFORMS_OPENGLES


//-------------------------------------------------------------------------------------------------
// Vulkan
//-------------------------------------------------------------------------------------------------

#if QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN

#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"

// moved from below
#define VULKAN_SEMAPHORE_EXPORT_TYPE VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR


#if 0
#define VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR				     (VkStructureType)1000059003
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR	     (VkStructureType)1000059004
#define VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR		     (VkStructureType)1000071001
#define VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR		     (VkStructureType)1000072001
#define VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR			     (VkStructureType)1000072002
#define VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR					     (VkStructureType)1000074002
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR    (VkStructureType)1000076000
#define VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR			     (VkStructureType)1000076001
#define VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR			     (VkStructureType)1000077000
#define VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR					     (VkStructureType)1000079001
#define VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR		     (VkStructureType)1000127001
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR (VkStructureType)1000071000

typedef enum VkExternalMemoryHandleTypeFlagBitsKHR {
	VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR = 0x00000001,
	VK_EXTERNAL_MEMORY_HANDLE_TYPE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkExternalMemoryHandleTypeFlagBitsKHR;

#define VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_KHR			0x00000001
#define VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR				0x00000002

typedef VkFlags VkExternalMemoryHandleTypeFlagsKHR;
typedef VkFlags VkExternalMemoryFeatureFlagsKHR;

typedef struct VkPhysicalDeviceExternalImageFormatInfoKHR {
	VkStructureType                          sType;
	const void*                              pNext;
	VkExternalMemoryHandleTypeFlagBitsKHR    handleType;
} VkPhysicalDeviceExternalImageFormatInfoKHR;

typedef struct VkPhysicalDeviceImageFormatInfo2KHR {
	VkStructureType       sType;
	const void*           pNext;
	VkFormat              format;
	VkImageType           type;
	VkImageTiling         tiling;
	VkImageUsageFlags     usage;
	VkImageCreateFlags    flags;
} VkPhysicalDeviceImageFormatInfo2KHR;

typedef struct VkImageFormatProperties2KHR {
	VkStructureType            sType;
	void*                      pNext;
	VkImageFormatProperties    imageFormatProperties;
} VkImageFormatProperties2KHR;

typedef struct VkExternalMemoryImageCreateInfoKHR {
	VkStructureType                       sType;
	const void*                           pNext;
	VkExternalMemoryHandleTypeFlagsKHR    handleTypes;
} VkExternalMemoryImageCreateInfoKHR;

typedef struct VkExternalMemoryPropertiesKHR {
	VkExternalMemoryFeatureFlagsKHR       externalMemoryFeatures;
	VkExternalMemoryHandleTypeFlagsKHR    exportFromImportedHandleTypes;
	VkExternalMemoryHandleTypeFlagsKHR    compatibleHandleTypes;
} VkExternalMemoryPropertiesKHR;

typedef struct VkExternalImageFormatPropertiesKHR {
	VkStructureType                  sType;
	void*                            pNext;
	VkExternalMemoryPropertiesKHR    externalMemoryProperties;
} VkExternalImageFormatPropertiesKHR;

typedef struct VkExportMemoryAllocateInfoKHR {
	VkStructureType                       sType;
	const void*                           pNext;
	VkExternalMemoryHandleTypeFlagsKHR    handleTypes;
} VkExportMemoryAllocateInfoKHR;

typedef struct VkMemoryDedicatedAllocateInfoKHR {
	VkStructureType    sType;
	const void*        pNext;
	VkImage            image;
	VkBuffer           buffer;
} VkMemoryDedicatedAllocateInfoKHR;

typedef struct VkMemoryGetFdInfoKHR {
	VkStructureType                          sType;
	const void*                              pNext;
	VkDeviceMemory                           memory;
	VkExternalMemoryHandleTypeFlagBitsKHR    handleType;
} VkMemoryGetFdInfoKHR;

typedef enum VkExternalSemaphoreHandleTypeFlagBitsKHR {
	VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR = 0x00000001,
	VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR = 0x00000010,
	VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkExternalSemaphoreHandleTypeFlagBitsKHR;
typedef VkFlags VkExternalSemaphoreHandleTypeFlagsKHR;

#define VULKAN_SEMAPHORE_EXPORT_TYPE VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR

typedef struct VkExportSemaphoreCreateInfoKHR {
	VkStructureType                          sType;
	const void*                              pNext;
	VkExternalSemaphoreHandleTypeFlagsKHR    handleTypes;
} VkExportSemaphoreCreateInfoKHR;

typedef struct VkPhysicalDeviceExternalSemaphoreInfoKHR {
	VkStructureType                             sType;
	const void*                                 pNext;
	VkExternalSemaphoreHandleTypeFlagBitsKHR    handleType;
} VkPhysicalDeviceExternalSemaphoreInfoKHR;

typedef enum VkExternalSemaphoreFeatureFlagBitsKHR {
	VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR = 0x00000001,
	VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR = 0x00000002,
	VK_EXTERNAL_SEMAPHORE_FEATURE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkExternalSemaphoreFeatureFlagBitsKHR;
typedef VkFlags VkExternalSemaphoreFeatureFlagsKHR;

typedef struct VkExternalSemaphorePropertiesKHR {
	VkStructureType                          sType;
	void*                                    pNext;
	VkExternalSemaphoreHandleTypeFlagsKHR    exportFromImportedHandleTypes;
	VkExternalSemaphoreHandleTypeFlagsKHR    compatibleHandleTypes;
	VkExternalSemaphoreFeatureFlagsKHR       externalSemaphoreFeatures;
} VkExternalSemaphorePropertiesKHR;

typedef struct VkSemaphoreGetFdInfoKHR {
	VkStructureType                             sType;
	const void*                                 pNext;
	VkSemaphore                                 semaphore;
	VkExternalSemaphoreHandleTypeFlagBitsKHR    handleType;
} VkSemaphoreGetFdInfoKHR;

typedef VkResult(VKAPI_PTR *PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2KHR* pImageFormatInfo, VkImageFormatProperties2KHR* pImageFormatProperties);
extern PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR;

typedef VkResult(VKAPI_PTR *PFN_vkGetMemoryFdKHR)(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd);
extern PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR;

typedef void (VKAPI_PTR *PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfoKHR* pExternalSemaphoreInfo, VkExternalSemaphorePropertiesKHR* pExternalSemaphoreProperties);
extern PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR vkGetPhysicalDeviceExternalSemaphorePropertiesKHR;

typedef VkResult(VKAPI_PTR *PFN_vkGetSemaphoreFdKHR)(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd);
extern PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR;

#endif // fred

#endif // QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN
