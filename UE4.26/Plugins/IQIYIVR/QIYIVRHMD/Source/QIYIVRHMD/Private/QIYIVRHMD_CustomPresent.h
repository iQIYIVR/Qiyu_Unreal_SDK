//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#pragma once

#define USING_OPENGL_PATHWAY 1

#include "IQIYIVRHMDModule.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSVR, Log, All);

#include "sxrApi.h"
#include "Containers/CircularBuffer.h"

//--------------------------------------------------------------------------------------------------------


class FQIYIVRHMDCustomPresent : public FRHICustomPresent
{
	friend class FViewExtension;
	friend class FQIYIVRHMD;

	enum
	{
		kMaxPoseHistoryLength = 8
	};

public:
	static FQIYIVRHMDCustomPresent* Create(class FQIYIVRHMD* pHMD);

	// FRHICustomPresent
	virtual void OnBackBufferResize() override;

	// Called from render thread to see if a native present will be requested for this frame.
	// @return	true if native Present will be requested for this frame; false otherwise.  Must
	// match value subsequently returned by Present for this frame.
	virtual bool NeedsNativePresent() override;

	// Called from RHI thread to perform custom present.
	// @param InOutSyncInterval - in out param, indicates if vsync is on (>0) or off (==0).
	// @return	true if native Present should be also be performed; false otherwise. If it returns
	// true, then InOutSyncInterval could be modified to switch between VSync/NoVSync for the normal 
	// Present.  Must match value previously returned by NeedsNormalPresent for this frame.
	virtual bool Present(int32& InOutSyncInterval) override;

	FQIYIVRHMDCustomPresent(class FQIYIVRHMD* pHMD);
	~FQIYIVRHMDCustomPresent();

	void BeginRendering(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily);
	void FinishRendering();
	void UpdateViewport(const FViewport& Viewport, FRHIViewport* InViewportRHI);
	void DoBeginVR();
	void DoEndVR();

	struct FPoseStateFrame
	{
		sxrHeadPoseState Pose;
		uint64 FrameNumber;
	};

	void CreateLayout(float centerX, float centerY, float radiusX, float radiusY, sxrLayoutCoords *pLayout);

	virtual void SubmitFrame(const FPoseStateFrame& PoseState) = 0;

	virtual bool AllocateRenderTargetTexture(
		uint32 SizeX,
		uint32 SizeY,
		uint8 Format,
		uint32 NumMips,
		uint32 Flags,
		uint32 TargetableTextureFlags,
		FTexture2DRHIRef& OutTargetableTexture,
		FTexture2DRHIRef& OutShaderResourceTexture,
		uint32 NumSamples) = 0;

	// TODO: Refactor
	inline bool UsesArrayTexture() const { return bUsesArrayTexture; }

	static FCriticalSection InVRModeCriticalSection;
	static FCriticalSection	PerfLevelCriticalSection;

protected:
	bool bInVRMode;
	bool bContextInitialized;

	// TODO: Refactor
	bool bUsesArrayTexture;
	bool bFirstInit;

	// This is the pose used to render the current frame (pass to SVR for warp)
	sxrHeadPoseState	mRenderPose;
};

FQIYIVRHMDCustomPresent* CreateCustomPresent_OpenGLES(class FQIYIVRHMD* pHMD);
FQIYIVRHMDCustomPresent* CreateCustomPresent_Vulkan(class FQIYIVRHMD* pHMD);

