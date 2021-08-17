//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#pragma once


#include "XRRenderTargetManager.h"
#include "IStereoLayers.h"
#include "IQIYIVRHMDModule.h"
#ifdef UE4_420
#include "XRRenderBridge.h"
#endif

#include "sxrApi.h"
#include "SceneViewExtension.h"
#include "QIYIVRHMD_CustomPresent.h"
#include "QiyiVRSDKCore.h"

//-----------------------------------------------------------------------------
class FQIYIVRHMD : public FHeadMountedDisplayBase, public FXRRenderTargetManager, public FSceneViewExtensionBase 
{
public:

	FQIYIVRHMD(const FAutoRegister&);
	virtual ~FQIYIVRHMD();

	static FQIYIVRHMD* GetQIYIHMD();

	static const FName QIYIVRHMDSystemName;

	void SetCPUAndGPULevels(const int32 InCPULevel, const int32 InGPULevel) const;
	virtual FRHICustomPresent* GetCustomPresent() { return pQIYIVRBridge; }
	bool IsInitialized() const;
	void UpdatePoses();
	void UpdateMyPoses(sxrHeadPoseState& HPS);
	void DrawDebug(class UCanvas* Canvas, class APlayerController*);
	void DrawDebugTrackingCameraFrustum(UWorld* World, const FRotator& ViewRotation, const FVector& ViewLocation);
	void ShutdownRendering();
	bool GetRelativeEyeDirection(int32 DeviceId, FVector& OutDirection);
	bool GetEyePoseState(int32 DeviceId, sxrEyePoseState& EyePoseState);
	void GetControllerState(QiyiVRCore::ControllerData &LeftData, QiyiVRCore::ControllerData &RightData);

	//////////////////////////////////////////////////////
	// IXRSystemIdentifier
	//////////////////////////////////////////////////////

	virtual FName GetSystemName() const override
	{
		return QIYIVRHMDSystemName;
	}

	//////////////////////////////////////////////////////
	// IXRTrackingSystem
	//////////////////////////////////////////////////////

	virtual void ResetPosition() override;
	virtual FString GetVersionString() const override;
	virtual bool DoesSupportPositionalTracking() const override;
	virtual bool HasValidTrackingPosition();
	virtual bool EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type = EXRTrackedDeviceType::Any) override;
	virtual bool GetCurrentPose(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition) override;
	virtual bool GetRelativeEyePose(int32 DeviceId, EStereoscopicPass Eye, FQuat& OutOrientation, FVector& OutPosition) override;
	virtual void ResetOrientationAndPosition(float Yaw = 0.f) override;
	virtual void ResetOrientation(float Yaw = 0.f) override;
	virtual void SetBaseRotation(const FRotator& BaseRot) override;
	virtual FRotator GetBaseRotation() const override;
	virtual void SetBaseOrientation(const FQuat& BaseOrient) override;
	virtual FQuat GetBaseOrientation() const override;
	virtual bool OnStartGameFrame(FWorldContext& WorldContext) override;
	virtual bool OnEndGameFrame(FWorldContext& WorldContext) override;
	virtual void OnBeginRendering_GameThread() override;
	virtual void OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily) override;
	virtual bool IsHeadTrackingAllowed() const override;
	virtual void OnBeginPlay(FWorldContext& InWorldContext) override;
	virtual void OnEndPlay(FWorldContext& InWorldContext) override;
	virtual class IHeadMountedDisplay* GetHMDDevice() override { return this; }
	virtual class TSharedPtr< class IStereoRendering, ESPMode::ThreadSafe > GetStereoRenderingDevice() override { return SharedThis(this); }
	virtual void SetTrackingOrigin(EHMDTrackingOrigin::Type InOrigin) override;
	virtual EHMDTrackingOrigin::Type GetTrackingOrigin() const override;

	//////////////////////////////////////////////////////
	// IHeadMountedDisplay
	//////////////////////////////////////////////////////
	virtual void UpdateScreenSettings(const FViewport* InViewport) override {}
	virtual bool HasHiddenAreaMesh() const override { return HiddenAreaMeshes[0].IsValid() && HiddenAreaMeshes[1].IsValid(); }
	virtual void DrawHiddenAreaMesh_RenderThread(FRHICommandList& RHICmdList, EStereoscopicPass StereoPass) const override;
	virtual bool HasVisibleAreaMesh() const override { return false; }
	virtual void DrawVisibleAreaMesh_RenderThread(FRHICommandList& RHICmdList, EStereoscopicPass StereoPass) const override;
	virtual void DrawDistortionMesh_RenderThread(struct FRenderingCompositePassContext& Context, const FIntPoint& TextureSize) override;
	virtual void GetEyeRenderParams_RenderThread(const struct FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
	//////////////////////////////////////////////////////
	//ISceneViewExtension
	//////////////////////////////////////////////////////

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override {}
	//virtual bool IsActiveThisFrame(class FViewport* InViewport) const override { return false;  }  // RBF questionable

	///////////////////////////////////////////////////
	// IStereoRendering 
	///////////////////////////////////////////////////
	virtual bool IsStereoEnabled() const override;
	virtual bool EnableStereo(bool stereo = true) override;
	virtual void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
	virtual FMatrix GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType) const override;
	virtual void SetFinalViewRect(const enum EStereoscopicPass StereoPass, const FIntRect& FinalViewRect) override;
	virtual void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize) const override;
	virtual IStereoRenderTargetManager* GetRenderTargetManager() override { return this; }

	////////////////////////////////////////////
	// Begin FXRRenderTargetManager
	////////////////////////////////////////////
	virtual void CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) override;
	virtual bool ShouldUseSeparateRenderTarget() const override;
	virtual bool AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 Flags, uint32 TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples = 1) override;

	//////////////////////////////////////////////////////
	// Begin IHeadMountedDisplay 
	//////////////////////////////////////////////////////
	virtual bool IsHMDConnected() override { return true; };
	virtual bool IsHMDEnabled() const override;
	virtual void EnableHMD(bool bEnable = true) override;
	virtual bool GetHMDMonitorInfo(MonitorInfo&) override;
	virtual void GetFieldOfView(float& InOutHFOVInDegrees, float& InOutVFOVInDegrees) const override;
	virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
	virtual float GetInterpupillaryDistance() const override;
	virtual bool IsChromaAbCorrectionEnabled() const override;
	virtual void CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation) override;
	virtual void InitCanvasFromView(FSceneView* InView, UCanvas* Canvas) override;
	virtual bool NeedReAllocateViewportRenderTarget(const FViewport& Viewport) override;
	virtual void UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& Viewport, SViewport*) override;
	virtual uint32 GetNumberOfBufferedFrames() const override;

public:

	/** FXRTrackingSystemBase protected interface */
	virtual float GetWorldToMetersScale() const override;

	// Performance helper functions
	static void PerfLevelLog(const TCHAR* const InPrefix, enum sxrPerfLevel InPerfLevelCpu, enum sxrPerfLevel InPerfLevelGpu);
	static bool PerfLevelsLastSetByCvarRead( enum sxrPerfLevel* OutPerfLevelCpuCurrent, enum sxrPerfLevel* OutPerfLevelGpuCurrent, 
											const enum sxrPerfLevel InPerfLevelCpuDefault,const enum sxrPerfLevel InPerfLevelGpuDefault);
	static enum sxrPerfLevel GetCVarQIYIVrPerfLevelDefault();
	static void PerfLevelCpuWrite(const enum sxrPerfLevel InPerfLevel);
	static void PerfLevelGpuWrite(const enum sxrPerfLevel InPerfLevel);
	static enum sxrPerfLevel PerfLevelCpuLastSet, PerfLevelGpuLastSet;

	// Eye tracking helper functions

	// pass in an sxrEyePoseState eye status member
	bool isEyeGazePointValid(int32_t& Status)      const { return  0 != (Status & sxrEyePoseStatus::kGazePointValid); }
	bool isEyeGazeDirectionValid(int32_t& Status)  const { return  0 != (Status & sxrEyePoseStatus::kGazeVectorValid); }
	bool isEyeOpenessValid(int32_t& Status)        const { return  0 != (Status & sxrEyePoseStatus::kEyeOpennessValid); }
	bool isEyePupilDialationValid(int32_t& Status) const { return  0 != (Status & sxrEyePoseStatus::kEyePupilDilationValid); }
	bool isEyePositionGuideValid(int32_t& Status)  const { return  0 != (Status & sxrEyePoseStatus::kEyePositionGuideValid); }

	bool isEyeTrackingEnabled() const;
	void enableEyeTracking(bool b);

	// get the cached eye pose
	const sxrEyePoseState&  GetLatestEyePoseState();
	// get the cached head pose
	bool GetHeadPoseState(sxrHeadPoseState& HeadPoseState);

	sxrHeadPoseState GetCachedHeadPoseState() { return CachedHeadPoseState; }
private:
	bool Startup();
	void InitializeIfNecessaryOnResume();
	void CleanupIfNecessary();

	void ApplicationWillEnterBackgroundDelegate();
	void ApplicationWillDeactivateDelegate();
	void ApplicationHasEnteredForegroundDelegate();
	void ApplicationHasReactivatedDelegate();

	void PoseToOrientationAndPosition(const sxrHeadPose& Pose, FQuat& CurrentOrientation, FVector& CurrentPosition, const float WorldToMetersScale);

	void BeginVRMode();
	void EndVRMode();

	void SendEvents();
	void SetupOcclusionMeshes();

	// wrapper around the raw device info - use this to get massaged values
	sxrDeviceInfo GetDeviceInfo();

	// these are similar to what Oculus has

	//  Converts vector from SXR space to Unreal
	//  transform position SVR to UE4 (-Z[2], X[0], Y[1])
	FORCEINLINE FVector SXR2Unreal_FVector(const sxrVector3& InVec)
	{
		return FVector(-InVec.z, InVec.x, InVec.y);
	}

	//  Converts vector from Unreal space to SXR 
	FORCEINLINE sxrVector3 Unreal2SXR_Vector(const FVector& InVec)
	{
		return sxrVector3{ InVec.Y, InVec.Z, -InVec.X };
	}

	// which view array index is this pass for?
	FORCEINLINE int32 ViewIndexFromStereoPass(const EStereoscopicPass StereoPassType) const
	{
		switch (StereoPassType)
			{
				case eSSP_LEFT_EYE:
				case eSSP_FULL:
					return 0;

				case eSSP_RIGHT_EYE:
					return 1;

				//case eSSP_MONOSCOPIC_EYE:
					//return 2;

				default:
					check(0);
					return -1;
		}
	}

private:
	bool bInitialized;
	bool bResumed;

	TRefCountPtr<FQIYIVRHMDCustomPresent> pQIYIVRBridge;


	bool InitializeExternalResources();

	//Temporary until sxr pose is integrated
	FQuat					CurHmdOrientation;
	FQuat					LastHmdOrientation;
	FVector                 CurHmdPosition;
	FVector					LastHmdPosition;
    FRotator				DeltaControlRotation;    
    FQuat					DeltaControlOrientation; 
	FQuat					BaseOrientation;
	double					LastSensorTime;
	sxrEyePoseState			CachedEyePoseState;
	sxrHeadPoseState		CachedHeadPoseState;

	FIntPoint				RenderTargetSize;
	FIntRect				EyeRenderViewport[2];
	FHMDViewMesh			HiddenAreaMeshes[2];

};

