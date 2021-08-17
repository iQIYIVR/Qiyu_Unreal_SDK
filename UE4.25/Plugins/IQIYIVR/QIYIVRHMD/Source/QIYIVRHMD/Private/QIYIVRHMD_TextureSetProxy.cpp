//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_TextureSetProxy.h"
#include "QIYIVR_CVars.h"

FQIYIVRTextureSet::FQIYIVRTextureSet()
{
	check(CVars::GNumSwapchainImages > 0);
	SwapChainImages.AddZeroed(CVars::GNumSwapchainImages);
}

FQIYIVRTextureSet::~FQIYIVRTextureSet()
{

}

void FQIYIVRTextureSet::SwitchToNextElement()
{
	SwapChainImageIndex = ((SwapChainImageIndex + 1) % CVars::GNumSwapchainImages);
	SetNativeResource(SwapChainImages[SwapChainImageIndex]);
}