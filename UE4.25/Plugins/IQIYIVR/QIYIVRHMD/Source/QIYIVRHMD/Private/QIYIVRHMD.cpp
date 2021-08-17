//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#include "QIYIVRHMD.h"

#include "../Public/QIYIVRHMDFunctionLibrary.h"

#include "../Classes/QIYIVRHMDEventManager.h"

#include "QIYIVR_CVars.h"

#include "QiyuSettings.h"

#include "RendererPrivate.h"
#include "ScenePrivate.h"
#include "Slate/SceneViewport.h"
#include "PostProcess/PostProcessHMD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "XRThreadUtils.h"
#include "HAL/RunnableThread.h"
#include "QIYIVRDataManager.h"

DECLARE_CYCLE_STAT(TEXT("Update Player Camera"), STAT_UpdatePlayerCamera, STATGROUP_Tickables);

#if PLATFORM_ANDROID

extern "C" SxrResult sxrInitializeOptArgs(const sxrInitParams* pInitParams, void* pTmAPI);

FCriticalSection FQIYIVRHMDCustomPresent::InVRModeCriticalSection;
FCriticalSection FQIYIVRHMDCustomPresent::PerfLevelCriticalSection;

#include <android_native_app_glue.h>

#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif // PLATFORM_ANDROID

#include "RHIStaticStates.h"
#include "Slate/SceneViewport.h"

#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogSVR);

#if QIYIVR_SUPPORTED_PLATFORMS

#include "Android/AndroidJNI.h"
#include <android_native_app_glue.h>
extern struct android_app* GNativeAndroidApp;
#endif 

static const TCHAR* const sxrPerfLevelSystemCStr = TEXT("sys");
static const TCHAR* const sxrPerfLevelMinimumCStr = TEXT("min");
static const TCHAR* const sxrPerfLevelMediumCStr = TEXT("med");
static const TCHAR* const sxrPerfLevelMaximumCStr = TEXT("max");

//keep these two lines in sync (eg pass the string that corresponds to the enum)
static const TCHAR* const sxrPerfLevelDefaultCStr = sxrPerfLevelSystemCStr;
/*static*/ enum sxrPerfLevel FQIYIVRHMD::GetCVarQIYIVrPerfLevelDefault() { return kPerfMedium; }

//these variables default values can be overridden by adding a line like this to Engine\Config\ConsoleVariables.ini: sxr.perfCpu=max
static TAutoConsoleVariable<FString> CVarPerfLevelCpu(TEXT("sxr.perfCpu"), sxrPerfLevelDefaultCStr, TEXT("Set QIYIVr CPU performance consumption to one of the following: sys, min, med, max.  Note that if this value is set by Blueprint, the cvar will continue to report the last value it was set to, and will not reflect the value set by Blueprint"));
static TAutoConsoleVariable<FString> CVarPerfLevelGpu(TEXT("sxr.perfGpu"), sxrPerfLevelDefaultCStr, TEXT("Set QIYIVr GPU performance consumption to one of the following: sys, min, med, max.  Note that if this value is set by Blueprint, the cvar will continue to report the last value it was set to, and will not reflect the value set by Blueprint"));

//BEG_Q3D_AUDIO_HACK
#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarQ3D_DrawSound(TEXT("q3d.drawSound"), 0, TEXT("Draws red tetrahedrons centered at each spatialized sound's position (hurts performance significantly)"));
static TAutoConsoleVariable<int32> CVarQ3D_DrawSoundName(TEXT("q3d.drawSoundName"), 0, TEXT("Draws red textual names of spatialized sounds near their positions (hurts performance dramatically)"));
static TAutoConsoleVariable<int32> CVarQ3D_LogSoundName(TEXT("q3d.logSoundName"), 0, TEXT("Logs sound names and positions of spatialized sounds each frame"));
static TAutoConsoleVariable<int32> CVarQ3D_LogSoundNumber(TEXT("q3d.logSoundNumber"), 0, TEXT("Logs the number of spatialized sounds playing on a frame"));
#endif//#if !UE_BUILD_SHIPPING
//END_Q3D_AUDIO_HACK




static void QIYIVRSdkCore_RequestMessageCallBack(QiyiVRCore::uint64 messageID, QiyiVRCore::PCChar result, QiyiVRCore::int32 resultSize)
{
	UE_LOG(LogTemp, Warning, TEXT("624Message: MessageCall01   "));
	
	//UE_LOG(LogTemp, Warning, TEXT("624Message: result:%s      "), *(FString)result);

	UQIYIVRDataManager::GetInstance()->MessageProcess(messageID,result,resultSize);

}

static void QIYIVRSdkCore_ListenerMessageCallBack(QiyiVRCore::uint64 messageID, QiyiVRCore::PCChar result, QiyiVRCore::int32 resultSize)
{
	UE_LOG(LogTemp, Warning, TEXT("624Message: ListenerMessage   "));

	//UE_LOG(LogTemp, Warning, TEXT("624Message: result:%s      "), *(FString)result);

	UQIYIVRDataManager::GetInstance()->ListenerProcess(messageID, result, resultSize);
}



//-----------------------------------------------------------------------------
static bool FStringToSvrPerfLevel(enum sxrPerfLevel* OutPerfLevel, const FString& InValueReceived)
{
    if (InValueReceived.Compare(FString(sxrPerfLevelSystemCStr, ESearchCase::IgnoreCase)) == 0)
    {
        *OutPerfLevel = kPerfSystem;
        return true;
    }
    else if (InValueReceived.Compare(FString(sxrPerfLevelMinimumCStr, ESearchCase::IgnoreCase)) == 0)
    {
        *OutPerfLevel = kPerfMinimum;
        return true;
    }
    else if (InValueReceived.Compare(FString(sxrPerfLevelMediumCStr, ESearchCase::IgnoreCase)) == 0)
    {
        *OutPerfLevel = kPerfMedium;
        return true;
    }
    else if (InValueReceived.Compare(FString(sxrPerfLevelMaximumCStr, ESearchCase::IgnoreCase)) == 0)
    {
        *OutPerfLevel = kPerfMaximum;
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
static bool SvrPerfLevelToFString(FString* const OutPerfLevelFString, const enum sxrPerfLevel InPerfLevel)
{
    switch (InPerfLevel)
    {
    case kPerfSystem:
    {
        *OutPerfLevelFString = FString(sxrPerfLevelSystemCStr);
        return true;
    }
    case kPerfMinimum:
    {
        *OutPerfLevelFString = FString(sxrPerfLevelMinimumCStr);
        return true;
    }
    case kPerfMedium:
    {
        *OutPerfLevelFString = FString(sxrPerfLevelMediumCStr);
        return true;
    }
    case kPerfMaximum:
    {
        *OutPerfLevelFString = FString(sxrPerfLevelMaximumCStr);
        return true;
    }
    default:
    {
        return false;
    }
    }
}

//-----------------------------------------------------------------------------
static bool IsPerfLevelValid(const enum sxrPerfLevel InPerfLevel)
{
    return InPerfLevel >= kPerfSystem && InPerfLevel <= kPerfMaximum;
}

// CTORNE ->
/*static*/ enum sxrPerfLevel FQIYIVRHMD::PerfLevelCpuLastSet = kNumPerfLevels, FQIYIVRHMD::PerfLevelGpuLastSet = kNumPerfLevels;
// <- CTORNE

//-----------------------------------------------------------------------------
/*static*/ void PerfLevelLastSet(enum sxrPerfLevel* const OutPerfLevel, const enum sxrPerfLevel InPerfLevel)
{
	check(IsPerfLevelValid(InPerfLevel));
	*OutPerfLevel = InPerfLevel;
}

//-----------------------------------------------------------------------------
/*static*/ void FQIYIVRHMD::PerfLevelCpuWrite(const enum sxrPerfLevel InPerfLevel)
{
	PerfLevelLastSet(&PerfLevelCpuLastSet, InPerfLevel);
}

//-----------------------------------------------------------------------------
/*static*/ void FQIYIVRHMD::PerfLevelGpuWrite(const enum sxrPerfLevel InPerfLevel)
{
	PerfLevelLastSet(&PerfLevelGpuLastSet, InPerfLevel);
}

//-----------------------------------------------------------------------------
/*static*/ void FQIYIVRHMD::PerfLevelLog(const TCHAR* const InPrefix,enum sxrPerfLevel InPerfLevelCpu, enum sxrPerfLevel InPerfLevelGpu)
{
#if !UE_BUILD_SHIPPING
    FString PerfLevelCpuLastSetFString, PerfLevelGpuLastSetFString;
    SvrPerfLevelToFString(&PerfLevelCpuLastSetFString, InPerfLevelCpu);
    SvrPerfLevelToFString(&PerfLevelGpuLastSetFString, InPerfLevelGpu);
    UE_LOG(LogSVR, Log, TEXT("%s:CPU = %s; GPU = %s"), InPrefix, *PerfLevelCpuLastSetFString, *PerfLevelGpuLastSetFString);
#endif//#if !UE_BUILD_SHIPPING
}

//-----------------------------------------------------------------------------
static bool PerfLevelLastSetByCvarRead(
	enum sxrPerfLevel* OutPerfLevel,
	const TAutoConsoleVariable<FString>& InCVar,
	const enum sxrPerfLevel InPerfLevelDefault)
{
	const bool bReadSucceeded = FStringToSvrPerfLevel(OutPerfLevel, InCVar.GetValueOnAnyThread());
	if (!bReadSucceeded)
	{
		*OutPerfLevel = InPerfLevelDefault;
	}

	return bReadSucceeded;
}

//-----------------------------------------------------------------------------
/*static*/ bool FQIYIVRHMD::PerfLevelsLastSetByCvarRead(
	enum sxrPerfLevel* OutPerfLevelCpuCurrent,
	enum sxrPerfLevel* OutPerfLevelGpuCurrent,
	const enum sxrPerfLevel InPerfLevelCpuDefault,
	const enum sxrPerfLevel InPerfLevelGpuDefault)
{
	bool bValid = PerfLevelLastSetByCvarRead(OutPerfLevelCpuCurrent, CVarPerfLevelCpu, InPerfLevelCpuDefault);
	bValid &= PerfLevelLastSetByCvarRead(OutPerfLevelGpuCurrent, CVarPerfLevelGpu, InPerfLevelGpuDefault);
	return bValid;
}

//-----------------------------------------------------------------------------
static void PerfLevelOnChangeByCvar(enum sxrPerfLevel* const OutPerfLevelToSet, const IConsoleVariable* const InConsoleVar, const TCHAR* const InLogPrefix)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	//sxr performance levels can be manipulated by render or game thread, so prevent race conditions
	FScopeLock ScopeLock(&FQIYIVRHMDCustomPresent::PerfLevelCriticalSection);

	enum sxrPerfLevel PerfLevel;
	const bool bReadSucceeded = FStringToSvrPerfLevel(&PerfLevel, InConsoleVar->GetString());
	if (bReadSucceeded)
	{
		PerfLevelLastSet(OutPerfLevelToSet, PerfLevel);
		FQIYIVRHMD::PerfLevelLog(InLogPrefix, FQIYIVRHMD::PerfLevelCpuLastSet, FQIYIVRHMD::PerfLevelGpuLastSet);
		sxrSetPerformanceLevels(FQIYIVRHMD::PerfLevelCpuLastSet, FQIYIVRHMD::PerfLevelGpuLastSet);
	}
#endif
}

//-----------------------------------------------------------------------------
static void PerfLevelCpuOnChangeByCvar(IConsoleVariable* InVar)
{
	PerfLevelOnChangeByCvar(&FQIYIVRHMD::PerfLevelCpuLastSet, InVar, TEXT("sxr.perfCpu cvar"));
}

//-----------------------------------------------------------------------------
static void PerfLevelGpuOnChangeByCvar(IConsoleVariable* InVar)
{
	PerfLevelOnChangeByCvar(&FQIYIVRHMD::PerfLevelGpuLastSet, InVar, TEXT("sxr.perfGpu cvar"));
}

const FName FQIYIVRHMD::QIYIVRHMDSystemName(TEXT("QIYIVRHMD"));


//-----------------------------------------------------------------------------
// FQIYIVRHMD, IHeadMountedDisplay Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Grab a pointer to the QIYI XR system
FQIYIVRHMD* FQIYIVRHMD::GetQIYIHMD()
{
	if (GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetSystemName() == FQIYIVRHMD::QIYIVRHMDSystemName)
	{
		return static_cast<FQIYIVRHMD*>(GEngine->XRSystem.Get());
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::IsHMDEnabled() const
{
    return true;
}
//-----------------------------------------------------------------------------
FString FQIYIVRHMD::GetVersionString() const
{
	FString s = FString::Printf(TEXT("QIYIVR - %s, built %s, %s"),
		*FEngineVersion::Current().ToString(), UTF8_TO_TCHAR(__DATE__), UTF8_TO_TCHAR(__TIME__));
	return s;
}
//-----------------------------------------------------------------------------
bool FQIYIVRHMD::HasValidTrackingPosition()
{
	// this actually tries to get one
	return DoesSupportPositionalTracking();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetFinalViewRect(const enum EStereoscopicPass StereoPass, const FIntRect& FinalViewRect)
{

}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type /*= EXRTrackedDeviceType::Any*/)
{
	if (Type == EXRTrackedDeviceType::Any || Type == EXRTrackedDeviceType::HeadMountedDisplay)
	{
		OutDevices.Add(IXRTrackingSystem::HMDDeviceId);
		UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::EnumerateTrackedDevices() - added device"));
		return true;
	}
	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::EnumerateTrackedDevices() - no device added"));
	return false;
 }

//-----------------------------------------------------------------------------
void  FQIYIVRHMD::UpdatePoses()
{
	// Don't update poses if on the render thread
	if (IsInRenderingThread() || IsInRHIThread())
	{
		return;
	}
	// UE_LOG(LogSVR, Error, TEXT("sxr (%s) (Frame %d) UpdatePoses() => Calling GetHeadPoseState"), IsInRenderingThread() ? TEXT("Render") : TEXT("Game"), GFrameNumber);
	// sxrprint("(%s) (Frame %d) UpdatePoses() => Calling GetHeadPoseState", IsInRenderingThread() ? TEXT("Render") : TEXT("Game"), GFrameNumber);

	GetHeadPoseState(CachedHeadPoseState);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::UpdateMyPoses(sxrHeadPoseState& Pose)
{
	GetHeadPoseState(Pose);
}

//-----------------------------------------------------------------------------
// rbf validate
bool FQIYIVRHMD::GetCurrentPose(int32 DeviceId, FQuat& CurrentOrientation, FVector& CurrentPosition)
{
	if (DeviceId != HMDDeviceId)
	{
		UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::GetCurrentPose() - Device error %d != %d"), DeviceId, HMDDeviceId);
		return false;
	}

	CurrentOrientation = CurHmdOrientation;
	CurrentPosition = CurHmdPosition;

	return true;
}

#pragma region EyeTracking

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::isEyeTrackingEnabled()const
{
	return  0 != CVars::EyeTrackingEnabled;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::enableEyeTracking(bool b)
{
	CVars::EyeTrackingEnabled = b ? 1 : 0;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::GetEyePoseState(int32 DeviceId, sxrEyePoseState& EyePoseState)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	if (DeviceId != HMDDeviceId)
	{
		UE_LOG(LogSVR, Error, TEXT("FQIYIVRHMD::GetEyePoseState() - Device error %d != %d"), DeviceId, HMDDeviceId);
		return false;
	}

	if (!pQIYIVRBridge->bInVRMode)
	{
		UE_LOG(LogSVR, Error, TEXT("FQIYIVRHMD::GetEyePoseState() - Not in VRMode"));
		return false;
	}

	if ((sxrGetSupportedTrackingModes() & kTrackingEye) == 0)
	{
		UE_LOG(LogSVR, Error, TEXT("FQIYIVRHMD::GetEyePoseState() - EyeTracking not available"));
		return false;
	}

	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::GetEyePoseState() from %s thread for frame:%d/%d"), IsInRenderingThread() ? TEXT("Render") : TEXT("Game"), GFrameNumber, GFrameNumberRenderThread);

	if (sxrGetEyePose(&EyePoseState) != SXR_ERROR_NONE)
	{	// if you get here...
		// sxrGetEyePose will write out the error into the log  - look for "sxrGetEyePose Failed" because the call to the service failed...
		UE_LOG(LogSVR, Error, TEXT("FQIYIVRHMD::GetEyePoseState - sxrGetEyePose Failed"));
		return false;
	}

	//UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::GetEyePoseState() says left position is %s and is [%6.3f,%6.3f,%6.3f]"),
	//	isEyeGazePointValid(EyePoseState.leftEyePoseStatus) ? TEXT("Valid") : TEXT("Invalid"), EyePoseState.leftEyeGazePoint[0], EyePoseState.leftEyeGazePoint[1], EyePoseState.leftEyeGazePoint[2]);
	//UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::GetEyePoseState() says left direction is %s and is [%6.3f,%6.3f,%6.3f]"),
	//	isEyeGazeDirectionValid(EyePoseState.leftEyePoseStatus) ? TEXT("Valid") : TEXT("Invalid"), EyePoseState.leftEyeGazeVector[0], EyePoseState.leftEyeGazeVector[1], EyePoseState.leftEyeGazeVector[2]);
	return true;
#endif
	return true;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::GetRelativeEyeDirection(int32 DeviceId, FVector& OutDirection)
{
	// Apply CS transform position SVR to UE4 (-Z[2], X[0], Y[1])
	FVector eyeDirections[3] =
	{
		FVector(-CachedEyePoseState.leftEyeGazeVector[2], CachedEyePoseState.leftEyeGazeVector[0], CachedEyePoseState.leftEyeGazeVector[1]),
		FVector(-CachedEyePoseState.rightEyeGazeVector[2], CachedEyePoseState.rightEyeGazeVector[0], CachedEyePoseState.rightEyeGazeVector[1]),
		FVector(-CachedEyePoseState.combinedEyeGazeVector[2], CachedEyePoseState.combinedEyeGazeVector[0], CachedEyePoseState.combinedEyeGazeVector[1])
	};

	bool bGazeVectorValidLeft  = isEyeGazeDirectionValid(CachedEyePoseState.leftEyePoseStatus);
	bool bGazeVectorValidRight = isEyeGazeDirectionValid(CachedEyePoseState.rightEyePoseStatus);
	bool bGazeVectorValidBoth  = isEyeGazeDirectionValid(CachedEyePoseState.combinedEyePoseStatus);

	// Currently, combined only valid if both left and right valid too 
	if (bGazeVectorValidBoth && bGazeVectorValidLeft && bGazeVectorValidRight) // Use combinedEyeDirection
	{
		OutDirection = eyeDirections[2];
		// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::GetRelativeEyeDirection combinedEyeDirection = x:%f, y:%f, z:%f"), OutDirection.X, OutDirection.Y, OutDirection.Z);
		return true;
	}
	else if ( bGazeVectorValidLeft || bGazeVectorValidRight )	// Average left/rightEyeDirection
	{
		OutDirection = FVector::ZeroVector;
		if (bGazeVectorValidLeft) OutDirection += eyeDirections[0];
		if (bGazeVectorValidRight) OutDirection += eyeDirections[1];
		OutDirection.Normalize();
		// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::GetRelativeEyeDirection averagedEyeDirection = x:%f, y:%f, z:%f"), OutDirection.X, OutDirection.Y, OutDirection.Z);
		return true;

		//dbgprint("FQIYIVRHMD::GetRelativeEyeDirection EyeDirection = x:%f, y:%f, z:%f", OutDirection.X, OutDirection.Y, OutDirection.Z);
	}
	return false;
}

//-----------------------------------------------------------------------------
// We don't really follow the instructions for this call....
bool FQIYIVRHMD::GetRelativeEyePose(int32 DeviceId, EStereoscopicPass Eye, FQuat& OutOrientation, FVector& OutPosition)
{
	bool rv = false;

	FVector combinedEyePosition = FVector::ZeroVector;
	FVector combinedEyeDirection = FVector::ZeroVector;

	rv = GetRelativeEyeDirection(DeviceId, combinedEyeDirection);

	if (rv)
	{
		OutPosition = combinedEyePosition;
		OutOrientation = combinedEyeDirection.ToOrientationQuat();
	}

	return rv;
}

//-----------------------------------------------------------------------------
const sxrEyePoseState& FQIYIVRHMD::GetLatestEyePoseState() // GetCurrentEyePose()
{
	return CachedEyePoseState;
}

#pragma endregion EyeTracking
//-----------------------------------------------------------------------------
void FQIYIVRHMD::EnableHMD(bool allow)
{
	EnableStereo(allow);
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
    if (IsInitialized())
    {
        MonitorDesc.MonitorName = "";
        MonitorDesc.MonitorId = 0;
        MonitorDesc.DesktopX = MonitorDesc.DesktopY = 0;
        MonitorDesc.ResolutionX = RenderTargetSize.X;
        MonitorDesc.ResolutionY = RenderTargetSize.Y;
        return true;
    }
    else
    {
		MonitorDesc.MonitorName = "";
        MonitorDesc.MonitorId = 0;
        MonitorDesc.DesktopX = MonitorDesc.DesktopY = MonitorDesc.ResolutionX = MonitorDesc.ResolutionY = 0;
    }

    return false;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
#if QIYIVR_SUPPORTED_PLATFORMS
    sxrDeviceInfo di = sxrGetDeviceInfo();
    OutHFOVInDegrees = FMath::RadiansToDegrees(di.targetFovXRad);
    OutVFOVInDegrees = FMath::RadiansToDegrees(di.targetFovYRad);
#endif
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::DoesSupportPositionalTracking() const
{
#if QIYIVR_SUPPORTED_PLATFORMS
    sxrDeviceInfo di = sxrGetDeviceInfo();
    const unsigned int supportedTrackingModes = sxrGetSupportedTrackingModes();

    const bool bTrackingPosition = supportedTrackingModes & kTrackingPosition;

    return bTrackingPosition;
#endif
	return false;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
	//UE_LOG(LogSVR, Log, TEXT("QIYIVR - call to SetInterpupillaryDistance is nop"));
    //Do Nothing
}

float FQIYIVRHMD::GetInterpupillaryDistance() const
{
	float value = 0.064f;
#if QIYIVR_SUPPORTED_PLATFORMS
	value = (float)QiyiVRCore::QVR_GetUserIPD() / 1000.0f;
#endif
	//UE_LOG(LogSVR, Log, TEXT("GetInterpupillaryDistance=%f"), value);
    return value;
}


// TSharedPtr<ISceneViewExtension, ESPMode::ThreadSafe> FQIYIVRHMD::GetViewExtension()
// {
//     TSharedPtr<FQIYIVRHMD, ESPMode::ThreadSafe> ptr(AsShared());
//     return StaticCastSharedPtr<ISceneViewExtension>(ptr);
// }


//-----------------------------------------------------------------------------
bool FQIYIVRHMD::IsChromaAbCorrectionEnabled() const
{
    return true;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::IsHeadTrackingAllowed() const
{
    return true;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ResetOrientationAndPosition(float yaw)
{
    ResetOrientation(yaw);
    ResetPosition();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ResetOrientation(float yaw)
{
	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ResetOrientation()"));
#if QIYIVR_SUPPORTED_PLATFORMS
	sxrRecenterOrientation();
#endif
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ResetPosition()
{
	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ResetPosition()"));
#if QIYIVR_SUPPORTED_PLATFORMS
	sxrRecenterPosition();
#endif
}

//-----------------------------------------------------------------------------
float FQIYIVRHMD::GetWorldToMetersScale() const
{
	return GWorld ? GWorld->GetWorldSettings()->WorldToMeters : 100.0f;  // RBF validate
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetBaseRotation(const FRotator& BaseRot)
{
	SetBaseOrientation(FRotator(0.0f, BaseRot.Yaw, 0.0f).Quaternion());
}

//-----------------------------------------------------------------------------
FRotator FQIYIVRHMD::GetBaseRotation() const
{
	return GetBaseOrientation().Rotator();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetBaseOrientation(const FQuat& BaseOrient)
{
	BaseOrientation = BaseOrient;
}

//-----------------------------------------------------------------------------
FQuat FQIYIVRHMD::GetBaseOrientation() const
{
	return BaseOrientation;
}

//-----------------------------------------------------------------------------
static void DrawOcclusionMesh(FRHICommandList& RHICmdList, EStereoscopicPass StereoPass, const FHMDViewMesh MeshAssets[])
{
	check(IsInRenderingThread() || IsInRHIThread());
	check(StereoPass != eSSP_FULL);
	const uint32 MeshIndex = (StereoPass == eSSP_LEFT_EYE) ? 0 : 1;
	const FHMDViewMesh& Mesh = MeshAssets[MeshIndex];
	check(Mesh.IsValid());

	RHICmdList.SetStreamSource(0, Mesh.VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(Mesh.IndexBufferRHI, 0, 0, Mesh.NumVertices, 0, Mesh.NumTriangles, 1);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::DrawHiddenAreaMesh_RenderThread(FRHICommandList& RHICmdList, EStereoscopicPass StereoPass) const
{
	DrawOcclusionMesh(RHICmdList, StereoPass, HiddenAreaMeshes);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::DrawVisibleAreaMesh_RenderThread(FRHICommandList& RHICmdList, EStereoscopicPass StereoPass) const
{
    //Do Nothing
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::DrawDistortionMesh_RenderThread(struct FRenderingCompositePassContext& Context, const FIntPoint& TextureSize)
{
    //Do Nothing
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::OnBeginPlay(FWorldContext& InWorldContext)
{
    UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::OnBeginPlay"));
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::OnEndPlay(FWorldContext& InWorldContext)
{
    UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::OnEndPlay"));
	///StopWebHelper();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetTrackingOrigin(EHMDTrackingOrigin::Type InOrigin)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	if (InOrigin == EHMDTrackingOrigin::Floor)
	{
		QiyiVRCore::QVR_SetFloorEyeType((int32)EFLOOREYE_TYPE::Floor);
	}
	if (InOrigin == EHMDTrackingOrigin::Eye)
	{
		QiyiVRCore::QVR_SetFloorEyeType((int32)EFLOOREYE_TYPE::Eye);
	}

#endif

	OnTrackingOriginChanged();
}

EHMDTrackingOrigin::Type FQIYIVRHMD::GetTrackingOrigin() const
{
	EHMDTrackingOrigin::Type rv = EHMDTrackingOrigin::Eye;

#if QIYIVR_SUPPORTED_PLATFORMS
	switch ( (EFLOOREYE_TYPE)QiyiVRCore::QVR_GetFloorEyeType() )
	{
	case EFLOOREYE_TYPE::Eye:
		rv = EHMDTrackingOrigin::Eye;
		break;
	case EFLOOREYE_TYPE::Floor:
		rv = EHMDTrackingOrigin::Floor;
		break;
	default:
		//UE_LOG(LogTemp, Error, TEXT("Unsupported ovr tracking origin type %d"), int((EFLOOREYE_TYPE)QiyiVRCore::QVR_GetFloorEyeType()));
		break;
	}

#endif
	return rv;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::OnStartGameFrame( FWorldContext& WorldContext )
{
	check(pQIYIVRBridge)
	check(IsInGameThread());
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_Update(0);
#endif
    //if sensor fusion is inactive, but the app has been resumed and initialized on both the main thread and graphics thread, then start sensor fusion
    if (bResumed && 
        pQIYIVRBridge->bContextInitialized && 
        !pQIYIVRBridge->bInVRMode &&
        bInitialized)
    {
		UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::OnStartGameFrame() - BeginVRMode"));
        BeginVRMode();
    }

	bool ReturnVal = true;  //  RBF cleanup


	UpdatePoses(); // GetHeadPoseState

	// cache the eye pose info if we're eyetracking
	if ( isEyeTrackingEnabled() )
	{
		GetEyePoseState(IXRTrackingSystem::HMDDeviceId, CachedEyePoseState);
	}

	RefreshTrackingToWorldTransform(WorldContext);

    return ReturnVal;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::OnEndGameFrame( FWorldContext& WorldContext )
{
	SendEvents();

    return true;
}

//-----------------------------------------------------------------------------
/**
 * Called on the game thread when view family is about to be rendered.
 */
void FQIYIVRHMD::OnBeginRendering_GameThread()
{
	 UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::OnBeginRendering_GameThread()"));
	 QiyiVRCore::ControllerData data1;
	 QiyiVRCore::ControllerData data2;
	 GetControllerState(data1, data2);
}

//-----------------------------------------------------------------------------
/**
 * Called on the render thread at the start of rendering.
 */
void FQIYIVRHMD::OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::OnBeginRendering_RenderThread()"));
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::GetHeadPoseState(sxrHeadPoseState& HeadPoseState)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	// Can only update the pose if vr has been started
	if (!pQIYIVRBridge->bInVRMode)
	{
		return false;
	}

	HeadPoseState = QiyiVRCore::QVR_GetHeadPoseState();

	FVector CurrentPosition;
	FQuat CurrentOrientation;
	PoseToOrientationAndPosition(HeadPoseState.pose, CurrentOrientation, CurrentPosition, GetWorldToMetersScale());
	CurHmdOrientation = LastHmdOrientation = BaseOrientation * CurrentOrientation;
	CurHmdPosition = LastHmdPosition = CurrentPosition;
#endif
	return true;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SendEvents()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	sxrEvent SxrEvent;
	while (sxrPollEvent(&SxrEvent))
	{
		switch (SxrEvent.eventType)
		{
		case sxrEventType::kEventSdkServiceStarting:
			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRSdkServiceDelegate.Broadcast(EQIYIVRSdkServiceEventType::EventSdkServiceStarting);
			break;
		case sxrEventType::kEventSdkServiceStarted:
			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRSdkServiceDelegate.Broadcast(EQIYIVRSdkServiceEventType::EventSdkServiceStarted);
			break;
		case sxrEventType::kEventSdkServiceStopped:
			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRSdkServiceDelegate.Broadcast(EQIYIVRSdkServiceEventType::EventSdkServiceStopped);
			break;
		case sxrEventType::kEventThermal:
		{
			EQIYIVRThermalZone ThermalZone = static_cast<EQIYIVRThermalZone>(SxrEvent.eventData.thermal.zone);
			EQIYIVRThermalLevel ThermalLevel = static_cast<EQIYIVRThermalLevel>(SxrEvent.eventData.thermal.level);

			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRThermalDelegate.Broadcast(ThermalZone, ThermalLevel);
			break;
		}
		case sxrEventType::kEventVrModeStarted:
			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRModeDelegate.Broadcast(EQIYIVRModeEventType::EventVrModeStarted);
			break;
		case sxrEventType::kEventVrModeStopping:
			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRModeDelegate.Broadcast(EQIYIVRModeEventType::EventVrModeStopping);
			break;
		case sxrEventType::kEventVrModeStopped:
			UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()->OnQIYIVRModeDelegate.Broadcast(EQIYIVRModeEventType::EventVrModeStopped);
			break;

		default:
			break;
		}
	}
#endif
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::DrawDebug(class UCanvas* Canvas, class APlayerController*)
{
    Canvas->DrawText(GEngine->GetLargeFont(), FText::Format( FText::FromString("Debug Draw {0}"), FText::AsNumber(GFrameNumber)), 10.f, 10.f);

	DrawDebugTrackingCameraFrustum(GWorld, Canvas->SceneView->ViewRotation, Canvas->SceneView->ViewLocation);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::DrawDebugTrackingCameraFrustum(UWorld* World, const FRotator& ViewRotation, const FVector& ViewLocation)
{
#if 0
	FPlatformMisc::LowLevelOutputDebugStringf(TEXT("FQIYIVRHMD::DrawDebugTrackingCameraFrustum"));

	const FColor c = (HasValidTrackingPosition() ? FColor::Green : FColor::Red);
	FVector origin;
	FQuat orient;
	float lfovDeg, rfovDeg, tfovDeg, bfovDeg, nearPlane, farPlane, cameraDist;
	uint32 nSensors = GetNumOfTrackingSensors();
	for (uint8 sensorIndex = 0; sensorIndex < nSensors; ++sensorIndex)
	{
		GetTrackingSensorProperties(sensorIndex, origin, orient, lfovDeg, rfovDeg, tfovDeg, bfovDeg, cameraDist, nearPlane, farPlane);

		FVector HeadPosition;
		FQuat HeadOrient;
		GetCurrentOrientationAndPosition(HeadOrient, HeadPosition);
		const FQuat DeltaControlOrientation = ViewRotation.Quaternion() * HeadOrient.Inverse();

		orient = DeltaControlOrientation * orient;

		origin = DeltaControlOrientation.RotateVector(origin);

		const float lfov = FMath::DegreesToRadians(lfovDeg);
		const float rfov = FMath::DegreesToRadians(rfovDeg);
		const float tfov = FMath::DegreesToRadians(tfovDeg);
		const float bfov = FMath::DegreesToRadians(bfovDeg);
		FVector coneTop(0, 0, 0);
		FVector coneBase1(-farPlane, farPlane * FMath::Tan(rfov), farPlane * FMath::Tan(tfov));
		FVector coneBase2(-farPlane, -farPlane * FMath::Tan(lfov), farPlane * FMath::Tan(tfov));
		FVector coneBase3(-farPlane, -farPlane * FMath::Tan(lfov), -farPlane * FMath::Tan(bfov));
		FVector coneBase4(-farPlane, farPlane * FMath::Tan(rfov), -farPlane * FMath::Tan(bfov));
		FMatrix m(FMatrix::Identity);
		m = orient * m;
		//m *= FScaleMatrix(frame->CameraScale3D);
		m *= FTranslationMatrix(origin);
		m *= FTranslationMatrix(ViewLocation); // to location of pawn
		coneTop = m.TransformPosition(coneTop);
		coneBase1 = m.TransformPosition(coneBase1);
		coneBase2 = m.TransformPosition(coneBase2);
		coneBase3 = m.TransformPosition(coneBase3);
		coneBase4 = m.TransformPosition(coneBase4);

		// draw a point at the camera pos
		DrawDebugPoint(World, coneTop, 5, c);

		// draw main pyramid, from top to base
		DrawDebugLine(World, coneTop, coneBase1, c);
		DrawDebugLine(World, coneTop, coneBase2, c);
		DrawDebugLine(World, coneTop, coneBase3, c);
		DrawDebugLine(World, coneTop, coneBase4, c);

		// draw base (far plane)				  
		DrawDebugLine(World, coneBase1, coneBase2, c);
		DrawDebugLine(World, coneBase2, coneBase3, c);
		DrawDebugLine(World, coneBase3, coneBase4, c);
		DrawDebugLine(World, coneBase4, coneBase1, c);

		// draw near plane
		FVector coneNear1(-nearPlane, nearPlane * FMath::Tan(rfov), nearPlane * FMath::Tan(tfov));
		FVector coneNear2(-nearPlane, -nearPlane * FMath::Tan(lfov), nearPlane * FMath::Tan(tfov));
		FVector coneNear3(-nearPlane, -nearPlane * FMath::Tan(lfov), -nearPlane * FMath::Tan(bfov));
		FVector coneNear4(-nearPlane, nearPlane * FMath::Tan(rfov), -nearPlane * FMath::Tan(bfov));
		coneNear1 = m.TransformPosition(coneNear1);
		coneNear2 = m.TransformPosition(coneNear2);
		coneNear3 = m.TransformPosition(coneNear3);
		coneNear4 = m.TransformPosition(coneNear4);
		DrawDebugLine(World, coneNear1, coneNear2, c);
		DrawDebugLine(World, coneNear2, coneNear3, c);
		DrawDebugLine(World, coneNear3, coneNear4, c);
		DrawDebugLine(World, coneNear4, coneNear1, c);

		// center line
		FVector centerLine(-cameraDist, 0, 0);
		centerLine = m.TransformPosition(centerLine);
		DrawDebugLine(World, coneTop, centerLine, FColor::Yellow);
		DrawDebugPoint(World, centerLine, 5, FColor::Yellow);
	}
#endif
}


//-----------------------------------------------------------------------------
// FQIYIVRHMD, IStereoRendering Implementation
//-----------------------------------------------------------------------------
bool FQIYIVRHMD::IsStereoEnabled() const
{
    return true;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::EnableStereo(bool stereo)
{

	GEngine->bForceDisableFrameRateSmoothing = stereo;
    return true;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{
	const int32 ViewIndex = ViewIndexFromStereoPass(StereoPass);

	X = EyeRenderViewport[ViewIndex].Min.X;
	Y = EyeRenderViewport[ViewIndex].Min.Y;
	SizeX = EyeRenderViewport[ViewIndex].Size().X;
	SizeY = EyeRenderViewport[ViewIndex].Size().Y;

}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::CalculateStereoViewOffset(const EStereoscopicPass StereoPassType, FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation)
{
    if( StereoPassType != eSSP_FULL)
    {
        const float EyeOffset = (GetInterpupillaryDistance() * 0.5f) * WorldToMeters;
        const float PassOffset = (StereoPassType == eSSP_LEFT_EYE) ? -EyeOffset : EyeOffset;
        ViewLocation += ViewRotation.Quaternion().RotateVector(FVector(0,PassOffset,0));

		//UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::CalculateStereoViewOffset x:%f, y:%f, z:%f"), ViewLocation.X, ViewLocation.Y, ViewLocation.Z);
    }
}

//-----------------------------------------------------------------------------
FMatrix FQIYIVRHMD::GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType) const
{
#if QIYIVR_SUPPORTED_PLATFORMS
	sxrDeviceInfo di = sxrGetDeviceInfo();

	sxrViewFrustum Frustum = (StereoPassType == eSSP_LEFT_EYE) ? di.leftEyeFrustum : di.rightEyeFrustum;

	float InvNearZ = 1.0f / Frustum.near;

	float Right = Frustum.right * InvNearZ;
	float Left = Frustum.left * InvNearZ;
	float Bottom = Frustum.bottom * InvNearZ;
	float Top = Frustum.top * InvNearZ;

	float ZNear = GNearClippingPlane;

	float SumRL = (Right + Left);
	float SumTB = (Top + Bottom);
	float InvRL = (1.0f / (Right - Left));
	float InvTB = (1.0f / (Top - Bottom));

	return FMatrix(
		FPlane((2.0f * InvRL), 0.0f, 0.0f, 0.0f),
		FPlane(0.0f, (2.0f * InvTB), 0.0f, 0.0f),
		FPlane((SumRL * InvRL), (SumTB * InvTB), 0.0f, 1.0f),
		FPlane(0.0f, 0.0f, ZNear, 0.0f)
	);
#endif
	return FMatrix();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::InitCanvasFromView(FSceneView* InView, UCanvas* Canvas)
{
    //Do Nothing
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
    EyeToSrcUVOffsetValue = FVector2D::ZeroVector;
    EyeToSrcUVScaleValue = FVector2D(1.0f, 1.0f);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY)
{
#if QIYIVR_SUPPORTED_PLATFORMS
    check(IsInGameThread());

    sxrDeviceInfo di = sxrGetDeviceInfo();

	// CTORNE: If using multiview direct, our 2d array rendertarget is actually single-wide
	uint32 ScalingFactor = ((FQIYIVRHMDCustomPresent*)GetCustomPresent())->UsesArrayTexture() ? 1 : 2;

    InOutSizeX = 1920/*di.targetEyeWidthPixels * ScalingFactor*/;
    InOutSizeY = 1080/*di.targetEyeHeightPixels*/;
	// CTORNE: This is temporary so we can experiment with resolution vs foveation settings in SpaceDock[SG].
	//		   Handling of VR resolution was changed in 4.18 so this will have to change too. Changing the
	//		   scale factor at runtime is untested.
	IConsoleVariable* CVarMobileContentScaleFactor = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MobileContentScaleFactor"));
	if (CVarMobileContentScaleFactor && !FMath::IsNearlyEqual(CVarMobileContentScaleFactor->GetFloat(), 1.f))
	{
		InOutSizeX = static_cast<uint32>(FMath::CeilToInt(CVarMobileContentScaleFactor->GetFloat() * InOutSizeX));
		InOutSizeY = static_cast<uint32>(FMath::CeilToInt(CVarMobileContentScaleFactor->GetFloat() * InOutSizeY));

		//dbgprint("FQIYIVRHMD::CalculateRenderTargetSize(): r.MobileContentScaleFactor = %f -> eye buffer size = %u x %u",
		//		 CVarMobileContentScaleFactor->GetFloat(), InOutSizeX, InOutSizeY);
	}
#endif
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::NeedReAllocateViewportRenderTarget(const FViewport& Viewport)
{
    check(IsInGameThread());

    const uint32 InSizeX = Viewport.GetSizeXY().X;
    const uint32 InSizeY = Viewport.GetSizeXY().Y;

    FIntPoint RTSize;
	RTSize.X = Viewport.GetRenderTargetTexture()->GetSizeX();
	RTSize.Y = Viewport.GetRenderTargetTexture()->GetSizeY();

    uint32 NewSizeX = InSizeX, NewSizeY = InSizeY;
    CalculateRenderTargetSize(Viewport, NewSizeX, NewSizeY);
    if (NewSizeX != RTSize.X || NewSizeY != RTSize.Y)
    {
		//UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::NeedReAllocateViewportRenderTarget returns true"));
		return true;
    }
    
	//UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::NeedReAllocateViewportRenderTarget returns false"));
	return false;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::ShouldUseSeparateRenderTarget() const
{

	//UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ShouldUseSeparateRenderTarget() called"));
    check(IsInGameThread());
    return IsStereoEnabled();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::UpdateViewport(bool bUseSeparateRenderTarget, const FViewport& InViewport, SViewport* ViewportWidget)
{
    check(IsInGameThread());
    FRHIViewport* const ViewportRHI = InViewport.GetViewportRHI().GetReference();

	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::UpdateViewport 1"));

    if (!IsStereoEnabled())
    {
        if (!bUseSeparateRenderTarget)
        {
            ViewportRHI->SetCustomPresent(nullptr);
			// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::UpdateViewport 2"));

        }
        return;
    }
	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::UpdateViewport 3"));
	pQIYIVRBridge->UpdateViewport(InViewport, ViewportRHI);
}

//-----------------------------------------------------------------------------
// FQIYIVRHMD, ISceneViewExtension Implementation
//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetupViewFamily(FSceneViewFamily& InViewFamily) 
{
    InViewFamily.EngineShowFlags.MotionBlur = 0;
    InViewFamily.EngineShowFlags.HMDDistortion = false;
    InViewFamily.EngineShowFlags.ScreenPercentage = false;
    InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();

	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::SetupViewFamily() and stereo is %s"), InViewFamily.EngineShowFlags.StereoRendering ? TEXT("enabled"):TEXT("disabled"));

}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
    InView.BaseHmdOrientation = LastHmdOrientation;
    InView.BaseHmdLocation = LastHmdPosition;
    //InViewFamily.bUseSeparateRenderTarget = true;
 
 	const int EyeIndex = (InView.StereoPass == eSSP_LEFT_EYE) ? 0 : 1;
 	//InView.ViewRect = EyeRenderViewport[EyeIndex];
	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::SetupView()"));

}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{

}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
	check(IsInRenderingThread() || IsInRHIThread());

	// RBF - I suspect this has to be rewritten

	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::PreRenderViewFamily_RenderThread()"));

    pQIYIVRBridge->BeginRendering(RHICmdList, InViewFamily);

}


//-----------------------------------------------------------------------------
// FQIYIVRHMD Implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FQIYIVRHMD Implementation
//-----------------------------------------------------------------------------
FQIYIVRHMD::FQIYIVRHMD(const FAutoRegister& AutoRegister) :
#if ENGINE_MINOR_VERSION >= 22
	FHeadMountedDisplayBase(nullptr),
#endif
	FSceneViewExtensionBase(AutoRegister),
	bInitialized(false),
    bResumed(false),
    CurHmdOrientation(FQuat::Identity),
    LastHmdOrientation(FQuat::Identity),
    DeltaControlRotation(FRotator::ZeroRotator),
    DeltaControlOrientation(FQuat::Identity),
	BaseOrientation(FQuat::Identity),
    CurHmdPosition(FVector::ZeroVector),
    LastHmdPosition(FVector::ZeroVector)
{

	memset(&CachedEyePoseState, 0, sizeof(CachedEyePoseState));
	memset(&CachedHeadPoseState, 0, sizeof(CachedHeadPoseState));

	// New for VR in 4.19
	static const auto PixelDensityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("vr.PixelDensity"));
	if (PixelDensityCVar)
	{
		PixelDensityCVar->Set(1.0f);
	}

	static const auto ScreenPercentageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	if ( ScreenPercentageCVar )
	{
		ScreenPercentageCVar->Set(100.0f);
	}
	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD()"));
	//UE_LOG(LogSVR, Log, TEXT("QVR_Init()   %p"), QiyiVRCore::QVR_Init);

#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_Init(
		FAndroidApplication::GetGameActivityThis(),
		true,
		//UQIYIVRDataManager::GetInstance()->MessageProcess,
		//UQIYIVRDataManager::GetInstance()->ListenerProcess
 		QIYIVRSdkCore_RequestMessageCallBack,
 		QIYIVRSdkCore_ListenerMessageCallBack
	);
	UE_LOG(LogTemp, Warning, TEXT("622Message:QVR_InitQiyuSDK point is （Unreal）:%d"),QIYIVRSdkCore_ListenerMessageCallBack);
#endif


	if (Startup())
	{
		SetupOcclusionMeshes();
	}
}

void FQIYIVRHMD::GetControllerState(QiyiVRCore::ControllerData &LeftData, QiyiVRCore::ControllerData &RightData)
{

#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_GetControllerDate(&LeftData, &RightData);
#endif
	UE_LOG(LogSVR, Log, TEXT("RightData Pos(%f,%f,%f)"), RightData.position.x, RightData.position.y, RightData.position.z);
}

//-----------------------------------------------------------------------------
FQIYIVRHMD::~FQIYIVRHMD()
{
    UE_LOG(LogSVR, Log, TEXT("~FQIYIVRHMD()"));
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::IsInitialized() const
{
	return bInitialized;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::InitializeExternalResources()
{
	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::InitializeExternalResources()"));

#if QIYIVR_SUPPORTED_PLATFORMS
	CVarPerfLevelCpu.AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&PerfLevelCpuOnChangeByCvar));
	CVarPerfLevelGpu.AsVariable()->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&PerfLevelGpuOnChangeByCvar));

    sxrInitParams initParams;

    initParams.javaVm = GJavaVM;
    initParams.javaEnv = FAndroidApplication::GetJavaEnv();
    initParams.javaActivityObject = FAndroidApplication::GetGameActivityThis();;

    if (sxrInitialize(&initParams) != SXR_ERROR_NONE)

    {
        UE_LOG(LogSVR, Error, TEXT("sxrInitialize failed"));
        return false;
    }
    else
    {
        UE_LOG(LogSVR, Log, TEXT("sxrInitialize succeeeded"));

		///StartWebHelper();
        uint32 TrackingMode = kTrackingRotation | kTrackingPosition;
        if (CVars::EyeTrackingEnabled)
        {
            if ((sxrGetSupportedTrackingModes() & kTrackingEye) == 0)
            {
				UE_LOG(LogSVR, Log, TEXT("Eye tracking is not supported on this device"));
                CVars::EyeTrackingEnabled = false;
            }
            else
            {
                TrackingMode |= kTrackingEye;
            }
        }
        sxrSetTrackingMode(TrackingMode);
        return true;
    }
#endif
	return false;
}

//-----------------------------------------------------------------------------
bool FQIYIVRHMD::Startup()
{
    UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::Startup(); this=%x"), this);
#if QIYIVR_SUPPORTED_PLATFORMS
	if (GRHISupportsRHIThread && GIsThreadedRendering && GUseRHIThread_InternalUseOnly)
	{
		SetRHIThreadEnabled(false, false);
	}

    bInitialized = InitializeExternalResources();
    if (!bInitialized)
    {
		UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::Startup() - Initialization FAILED"), this);
        return false;
    }

	pQIYIVRBridge = FQIYIVRHMDCustomPresent::Create(this);

    //Register life cycle delegates
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &FQIYIVRHMD::ApplicationWillEnterBackgroundDelegate);
    FCoreDelegates::ApplicationWillDeactivateDelegate.AddRaw(this, &FQIYIVRHMD::ApplicationWillDeactivateDelegate);//calls to this delegate are often (always?) paired with a call to ApplicationWillEnterBackgroundDelegate(), but cover the possibility that only this delegate is called
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &FQIYIVRHMD::ApplicationHasEnteredForegroundDelegate);
    FCoreDelegates::ApplicationHasReactivatedDelegate.AddRaw(this, &FQIYIVRHMD::ApplicationHasReactivatedDelegate);//calls to this delegate are often (always?) paired with a call to ApplicationHasEnteredForegroundDelegate(), but cover the possibility that only this delegate is called

    //don't bother with ApplicationWillTerminateDelegate() -- "There is no guarantee that this will ever be called on a mobile device, save state when ApplicationWillEnterBackgroundDelegate is called instead." -- https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Misc/FCoreDelegates/ApplicationWillTerminateDelegate/index.html --June 10, 2016
    //OnExit() and OnPreExit() are not documented as being called on mobile -- https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Misc/FCoreDelegates/OnExit/index.html and https://docs.unrealengine.com/latest/INT/API/Runtime/Core/Misc/FCoreDelegates/OnPreExit/index.html --June 10, 2016

	static const auto  CVarFrustumPositionLeft = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SXR.ViewFrustumLeft"));
	FString Value = CVarFrustumPositionLeft->GetString();
	UE_LOG(LogSVR, Log, TEXT("------------>>>>>>>>>>>>>>>>> ViewFrustumLeft %s"), *Value);


//	UE_LOG(LogSVR, Log, TEXT("------------>>>>>>>>>>>>>>>>> ViewFrustumLeft [%f %f %f] [%f %f %f %f]"), f[0], f[1], f[2], f[3], f[4], f[5], f[6]);

	sxrDeviceInfo DeviceInfo = sxrGetDeviceInfo();
	RenderTargetSize.X = 1920/*DeviceInfo.targetEyeWidthPixels * 2*/;
	RenderTargetSize.Y = 1080/*DeviceInfo.targetEyeHeightPixels*/;
// CTORNE ->
	// CTORNE: This is temporary so we can experiment with resolution vs foveation settings in SpaceDock[SG].
	//		   Handling of VR resolution was changed in 4.18 so this will have to change too. Changing the
	//		   scale factor at runtime is untested. 
	IConsoleVariable* CVarMobileContentScaleFactor = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MobileContentScaleFactor"));
	if (CVarMobileContentScaleFactor && !FMath::IsNearlyEqual(CVarMobileContentScaleFactor->GetFloat(), 1.f))
	{
		RenderTargetSize.X = static_cast<uint32>(FMath::CeilToInt(CVarMobileContentScaleFactor->GetFloat() * RenderTargetSize.X));
		RenderTargetSize.Y = static_cast<uint32>(FMath::CeilToInt(CVarMobileContentScaleFactor->GetFloat() * RenderTargetSize.Y));

		UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::Startup(): r.MobileContentScaleFactor = %f -> eye buffer size is now = %u x %u"),
			CVarMobileContentScaleFactor->GetFloat(), RenderTargetSize.X, RenderTargetSize.Y);
	}
// <- CTORNE

	const int32 RTSizeX = RenderTargetSize.X;
	const int32 RTSizeY = RenderTargetSize.Y;
	EyeRenderViewport[0] = FIntRect(0, 0, RTSizeX / 2 - 1, RTSizeY - 1);
	EyeRenderViewport[1] = FIntRect(RTSizeX / 2 + 1, 1, RTSizeX - 1, RTSizeY - 1);

	// disable vsync
	static IConsoleVariable* CVSyncVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync"));
	CVSyncVar->Set(false);

	// enforce finishcurrentframe
	static IConsoleVariable* CFCFVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FinishCurrentFrame"));
	CFCFVar->Set(false);

	// eye tracking
	static IConsoleVariable* CETEVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SVR.EyeTrackingEnabled"));
	CETEVar->Set(true);

	return true;
#endif
	return false;
}


//-----------------------------------------------------------------------------
void FQIYIVRHMD::CleanupIfNecessary()
{
#if QIYIVR_SUPPORTED_PLATFORMS
    bResumed = false;

    if (pQIYIVRBridge->bInVRMode)
    {
        UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::CleanupIfNecessary(): sxrEndXr() sxrShutdown()"));

        //stop consuming resources for VR until the app is resumed
        sxrEndXr();
        //sxrShutdown();
        pQIYIVRBridge->bInVRMode = false;
        bInitialized = false;
    }
#endif
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ApplicationWillEnterBackgroundDelegate()
{
	UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ApplicationWillEnterBackgroundDelegate"));
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_OnApplicationPause(true);
#endif
	CleanupIfNecessary();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ApplicationWillDeactivateDelegate()
{
    UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ApplicationWillDeactivateDelegate"));
    CleanupIfNecessary();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ApplicationHasReactivatedDelegate()
{
    UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ApplicationHasReactivatedDelegate"));
    InitializeIfNecessaryOnResume();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::ApplicationHasEnteredForegroundDelegate()
{
    UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::ApplicationHasEnteredForegroundDelegate"));
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_OnApplicationPause(false);
#endif
    InitializeIfNecessaryOnResume();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::InitializeIfNecessaryOnResume()
{
    if (!bInitialized)//Upon application launch, FQIYIVRHMD::Startup() must initialize immediately, but Android lifecycle "resume" delegates will also be called -- don't double-initialize
    {
		bInitialized = InitializeExternalResources();
		UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::InitializeIfNecessaryOnResume()"));
    }
    bResumed = true;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetCPUAndGPULevels(const int32 InCPULevel, const int32 InGPULevel) const
{
#if QIYIVR_SUPPORTED_PLATFORMS
	const sxrPerfLevel SvrPerfLevelCPU = static_cast<const sxrPerfLevel>(InCPULevel);
	const sxrPerfLevel SvrPerfLevelGPU = static_cast<const sxrPerfLevel>(InGPULevel);
	check(IsPerfLevelValid(SvrPerfLevelCPU));
	check(IsPerfLevelValid(SvrPerfLevelGPU));

	//sxr performance levels can be manipulated by render or game thread, so prevent race conditions
	FScopeLock ScopeLock(&pQIYIVRBridge->PerfLevelCriticalSection);
	PerfLevelCpuLastSet = SvrPerfLevelCPU;
	PerfLevelGpuLastSet = SvrPerfLevelGPU;

	if (pQIYIVRBridge->bInVRMode)
	{
		sxrSetPerformanceLevels(PerfLevelCpuLastSet, PerfLevelGpuLastSet);
		FQIYIVRHMD::PerfLevelLog(TEXT("SetCPUAndGPULevels"), PerfLevelCpuLastSet, PerfLevelGpuLastSet);
	}
#endif

}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::PoseToOrientationAndPosition(const sxrHeadPose& Pose, FQuat& OutCurrentOrientation, FVector& OutCurrentPosition, const float WorldToMetersScale)
{
	// UE_LOG(LogSVR, Log, TEXT("FQIYIVRHMD::PoseToOrientationAndPosition()"));

    if (pQIYIVRBridge->bInVRMode)
    {
		OutCurrentOrientation = FQuat(-Pose.rotation.z, Pose.rotation.x, Pose.rotation.y, Pose.rotation.w);
		OutCurrentOrientation = BaseOrientation.Inverse() * OutCurrentOrientation;
		OutCurrentOrientation.Normalize();	
		OutCurrentPosition = FVector(Pose.position.z * WorldToMetersScale, -Pose.position.x * WorldToMetersScale, -Pose.position.y * WorldToMetersScale);
		OutCurrentPosition = BaseOrientation.Inverse().RotateVector(OutCurrentPosition);
    }
    else
    {
        OutCurrentOrientation = FQuat(FRotator(0.0f, 0.0f, 0.0f));
        OutCurrentPosition = FVector(0.0f, 0.0f, 0.0f);
    }
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::BeginVRMode()
{
    check(pQIYIVRBridge);

	ExecuteOnRenderThread([this]()
	{
		ExecuteOnRHIThread([this]()
		{
			if (this->IsInitialized())
			{
				pQIYIVRBridge->DoBeginVR();
			}
		});
	});

	FlushRenderingCommands();
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::EndVRMode()
{
}

//-----------------------------------------------------------------------------
uint32 FQIYIVRHMD::GetNumberOfBufferedFrames() const
{
    return 1;
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SetupOcclusionMeshes()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	int TriangleCount = 0;
	int VertexStride = 0;
	if (sxrGetOcclusionMesh(kLeftEye, &TriangleCount, &VertexStride, nullptr) != SXR_ERROR_NONE)
	{
		return;
	}

	const uint32 VertexCount = TriangleCount * 3;

	if (VertexCount > 0 && VertexStride > 0)
	{
		const uint32 VertexDataSize = VertexCount * VertexStride;

		float* const LeftEyeMesh = new float[VertexDataSize];
		float* const RightEyeMesh = new float[VertexDataSize];
		sxrGetOcclusionMesh(kLeftEye, &TriangleCount, &VertexStride, LeftEyeMesh);
		sxrGetOcclusionMesh(kRightEye, &TriangleCount, &VertexStride, RightEyeMesh);

		FVector2D* const LeftEyePositions = new FVector2D[VertexCount];
		FVector2D* const RightEyePositions = new FVector2D[VertexCount];

		uint32 SourceIndex = 0;
		uint32 DestIndex = 0;
		for (int32 TriangleIter = 0; TriangleIter < TriangleCount; ++TriangleIter)
		{
			for (uint32 VertexIter = 0; VertexIter < 3; ++VertexIter)
			{
				const float* LeftSrc = &LeftEyeMesh[SourceIndex];
				const float* RightSrc = &RightEyeMesh[SourceIndex];

				FVector2D& LeftDst = LeftEyePositions[DestIndex];
				FVector2D& RightDst = RightEyePositions[DestIndex];

				LeftDst.X = (LeftSrc[0] + 1.f) * 0.5f;
				LeftDst.Y = (LeftSrc[1] + 1.f) * 0.5f;

				RightDst.X = (RightSrc[0] + 1.f) * 0.5f;
				RightDst.Y = (RightSrc[1] + 1.f) * 0.5f;

				SourceIndex += VertexStride;
				DestIndex++;
			}
		}

		HiddenAreaMeshes[0].BuildMesh(LeftEyePositions, VertexCount, FHMDViewMesh::MT_HiddenArea);
		HiddenAreaMeshes[1].BuildMesh(RightEyePositions, VertexCount, FHMDViewMesh::MT_HiddenArea);

		delete[] LeftEyePositions;
		delete[] RightEyePositions;

		delete[] LeftEyeMesh;
		delete[] RightEyeMesh;
	}
#endif
}

//-----------------------------------------------------------------------------
sxrDeviceInfo FQIYIVRHMD::GetDeviceInfo()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	sxrDeviceInfo di = sxrGetDeviceInfo();
	
	// if any coordinate system transformations are necessary, they should go here

	return di;
#endif
	return sxrDeviceInfo();
}

void FQIYIVRHMD::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize) const
{

}


bool FQIYIVRHMD::AllocateRenderTargetTexture(
	uint32 Index,
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
	return pQIYIVRBridge->AllocateRenderTargetTexture(
		SizeX,
		SizeY,
		Format,
		NumMips,
		Flags,
		TargetableTextureFlags,
		OutTargetableTexture,
		OutShaderResourceTexture,
		NumSamples);
}

#if 0 // webhelper stuff
//-----------------------------------------------------------------------------
void FQIYIVRHMD::StartWebHelper()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jstring StringArg1 = Env->NewStringUTF("SpaceDock");
	jstring StringArg2 = Env->NewStringUTF("score");
	jstring StringArg3 = Env->NewStringUTF("0");

	jclass FoundClass = FAndroidApplication::FindJavaClass("com/qualcomm/acg/webhelper/WebServiceStarter");
	FAndroidApplication::CheckJavaException();
	//UE_LOG(LogSVR, Log, TEXT("StartWebHelper after FoundClass  %x "), FoundClass);
	webHelperClass = FoundClass;

	jmethodID methodid = Env->GetStaticMethodID(FoundClass, "start", "(Ljava/lang/String;Landroid/app/Activity;)V");
	//UE_LOG(LogSVR, Log, TEXT("StartWebHelper after GetStaticMethodID : start  %x "), methodid);

	Env->CallStaticVoidMethod(FoundClass, methodid, StringArg1, FAndroidApplication::GetGameActivityThis());
	//UE_LOG(LogSVR, Log, TEXT("StartWebHelper after CallStaticVoidMethod :start"));

	webHelperSendScore = Env->GetStaticMethodID(FoundClass, "sendMessage", "(Ljava/lang/String;Ljava/lang/String;)V");
	UE_LOG(LogSVR, Log, TEXT("StartScore start=%x sendMessage=%x "), webHelperClass, webHelperSendScore);

	//Env->CallStaticVoidMethod(FoundClass, webHelperSendScore, StringArg2, StringArg3);
	//UE_LOG(LogSVR, Log, TEXT("StartWebHelper after CallStaticVoidMethod sendMessage"));

	Env->DeleteLocalRef(StringArg1);
	Env->DeleteLocalRef(StringArg2);
	Env->DeleteLocalRef(StringArg3);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::SendMessage(const int32 score) const
{
	char buf[64];
	sprintf(buf, "%d", score);
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jstring StringArg1 = Env->NewStringUTF(buf);
	jstring StringArg2 = Env->NewStringUTF("score");

	Env->CallStaticVoidMethod(webHelperClass, webHelperSendScore, StringArg2, StringArg1);
	UE_LOG(LogSVR, Log, TEXT("Score: After SendMessage %d"), score);

	Env->DeleteLocalRef(StringArg1);
	Env->DeleteLocalRef(StringArg2);
}

//-----------------------------------------------------------------------------
void FQIYIVRHMD::StopWebHelper()
{

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jmethodID methodid = Env->GetStaticMethodID(webHelperClass, "stop", "()V");
	//UE_LOG(LogSVR, Log, TEXT("StopWebHelper after GetStaticMethodID : stop  %x "), methodid);

	Env->CallStaticVoidMethod(webHelperClass, methodid);
	UE_LOG(LogSVR, Log, TEXT("StopScore after :stop"));

}

#endif // webhelper

