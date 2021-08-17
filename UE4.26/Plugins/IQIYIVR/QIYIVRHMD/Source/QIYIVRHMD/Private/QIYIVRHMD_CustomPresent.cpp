//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_CustomPresent.h"
#include "Runtime/Core/Public/Misc/ScopeLock.h"
#include "QIYIVRHMD.h"

#define dbgprint(format, ...) //FPlatformMisc::LowLevelOutputDebugStringf(*FString::Printf(TEXT("%s line %d: %s"), TEXT(__FILE__), __LINE__, *FString::Printf(TEXT(format), ##__VA_ARGS__)))


#if QIYIVR_SUPPORTED_PLATFORMS
#include <android_native_app_glue.h>
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "Android/AndroidEGL.h"
#include "Android/AndroidOpenGL.h"
#include "Android/AndroidApplication.h"
#endif

#include "HAL/IConsoleManager.h"


void FQIYIVRHMDCustomPresent::OnBackBufferResize()
{

}

bool FQIYIVRHMDCustomPresent::NeedsNativePresent()
{
	return false;
}

bool FQIYIVRHMDCustomPresent::Present(int32& InOutSyncInterval)
{
	// CTORNE: Check was removed from Release4.17, should it be here?
	check(IsInRenderingThread() || IsInRHIThread());

	FinishRendering();

	return false;
}

FQIYIVRHMDCustomPresent::FQIYIVRHMDCustomPresent(FQIYIVRHMD* pHMD)
	: FRHICustomPresent()
	, bInVRMode(false)
	, bContextInitialized(false)
	, bUsesArrayTexture(false)
	, bFirstInit(false)
{
}

FQIYIVRHMDCustomPresent::~FQIYIVRHMDCustomPresent()
{

}

void FQIYIVRHMDCustomPresent::BeginRendering(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	bContextInitialized = true;

	if (bInVRMode)
	{

		#if QIYIVR_SUPPORTED_PLATFORMS
		float PredictedTime = sxrGetPredictedDisplayTime();
		mRenderPose = sxrGetPredictedHeadPose(PredictedTime);
		#endif
		sxrBeginEye(kLeftEye);
		//sxrBeginEye(kRightEye);
	}
#endif
}

FQIYIVRHMDCustomPresent* FQIYIVRHMDCustomPresent::Create(FQIYIVRHMD* pHMD)
{
	if (FCString::Stricmp(GDynamicRHI->GetName(), TEXT("Vulkan")) == 0)
	{
		#if QIYIVR_HMD_SUPPORTED_PLATFORMS_VULKAN
		return CreateCustomPresent_Vulkan(pHMD);
		#endif
		return nullptr;
	}

	return CreateCustomPresent_OpenGLES(pHMD);
}


void FQIYIVRHMDCustomPresent::FinishRendering()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	check(IsInRenderingThread() || IsInRHIThread());

	if (bInVRMode)
	{
		FPoseStateFrame PoseState;
		PoseState.Pose = mRenderPose;
		PoseState.FrameNumber = GFrameNumber;
		//UpdateMyPoses(PoseState);

		sxrEndEye(sxrWhichEye::kLeftEye);
		sxrEndEye(sxrWhichEye::kRightEye);

		SubmitFrame(PoseState);
	}
#endif
}

void FQIYIVRHMDCustomPresent::UpdateViewport(const FViewport& Viewport, FRHIViewport* InViewportRHI)
{
	check(IsInGameThread());
	check(InViewportRHI);

	InViewportRHI->SetCustomPresent(this);
}

void FQIYIVRHMDCustomPresent::DoBeginVR()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	check(IsInRenderingThread() || IsInRHIThread());
	if (!bInVRMode)
	{
		bInVRMode = true;

		//sxr performance levels can be manipulated by render or game thread, so prevent race conditions
		FScopeLock ScopeLock(&PerfLevelCriticalSection);
		sxrBeginParams beginParams;
		memset(&beginParams, 0, sizeof(sxrBeginParams));

		if (FQIYIVRHMD::PerfLevelCpuLastSet != kNumPerfLevels && FQIYIVRHMD::PerfLevelGpuLastSet != kNumPerfLevels)
		{
			beginParams.cpuPerfLevel = FQIYIVRHMD::PerfLevelCpuLastSet;
			beginParams.gpuPerfLevel = FQIYIVRHMD::PerfLevelGpuLastSet;
		}
		else
		{
			const sxrPerfLevel PerfLevelDefault = FQIYIVRHMD::GetCVarQIYIVrPerfLevelDefault();
			FQIYIVRHMD::PerfLevelsLastSetByCvarRead(&beginParams.cpuPerfLevel, &beginParams.gpuPerfLevel, PerfLevelDefault, PerfLevelDefault);
		}

		beginParams.nativeWindow = AndroidEGL::GetInstance()->GetNativeWindow();
		beginParams.mainThreadId = gettid();

		FQIYIVRHMD::PerfLevelCpuWrite(beginParams.cpuPerfLevel);
		FQIYIVRHMD::PerfLevelGpuWrite(beginParams.gpuPerfLevel);
		FQIYIVRHMD::PerfLevelLog(TEXT("sxrBeginXr"), FQIYIVRHMD::PerfLevelCpuLastSet, FQIYIVRHMD::PerfLevelGpuLastSet);

		beginParams.colorSpace = kColorSpaceLinear;

		if (sxrBeginXr(&beginParams) != SXR_ERROR_NONE)
		{
			bInVRMode = false;
		}

		if (bFirstInit == false)
		{
			UE_LOG(LogSVR, Log, TEXT("bFirstInit=%d"), bFirstInit);
			sxrRecenterPose();
			bFirstInit = true;
		}
		
	}
#endif
}

void FQIYIVRHMDCustomPresent::DoEndVR()
{

}

void FQIYIVRHMDCustomPresent::CreateLayout(float centerX, float centerY, float radiusX, float radiusY, sxrLayoutCoords *pLayout)
{
	// This is always in screen space so we want Z = 0 and W = 1
	float lowerLeftPos[4] = { centerX - radiusX, centerY - radiusY, 0.0f, 1.0f };
	float lowerRightPos[4] = { centerX + radiusX, centerY - radiusY, 0.0f, 1.0f };
	float upperLeftPos[4] = { centerX - radiusX, centerY + radiusY, 0.0f, 1.0f };
	float upperRightPos[4] = { centerX + radiusX, centerY + radiusY, 0.0f, 1.0f };

	float lowerUVs[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float upperUVs[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

	memcpy(pLayout->LowerLeftPos, lowerLeftPos, sizeof(lowerLeftPos));
	memcpy(pLayout->LowerRightPos, lowerRightPos, sizeof(lowerRightPos));
	memcpy(pLayout->UpperLeftPos, upperLeftPos, sizeof(upperLeftPos));
	memcpy(pLayout->UpperRightPos, upperRightPos, sizeof(upperRightPos));

	memcpy(pLayout->LowerUVs, lowerUVs, sizeof(lowerUVs));
	memcpy(pLayout->UpperUVs, upperUVs, sizeof(upperUVs));

	float identTransform[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };
	memcpy(pLayout->TransformMatrix, identTransform, sizeof(pLayout->TransformMatrix));
}

