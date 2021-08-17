//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "../Public/QIYIVRHMDFunctionLibrary.h"
#include "QIYIVRHMD.h"
#include "QIYIVR_CVars.h"
#include "QiyuSettings.h"

//-----------------------------------------------------------------------------
UQIYIVRHMDFunctionLibrary::UQIYIVRHMDFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}


//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::SetCpuAndGpuLevelsSVR(const int32 CpuLevel, const int32 GpuLevel)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();

	// This is probably how we get the system w/o using engine
	typedef TSharedPtr< FQIYIVRHMD, ESPMode::ThreadSafe > FQIYIVRHMDPtr;
	// FQIYIVRHMDPtr FQIYIVRHMDp = FSceneViewExtensions::NewExtension<FQIYIVRHMD::FQIYIVRHMD>();

	if (HMD)
	{
		HMD->SetCPUAndGPULevels(CpuLevel, GpuLevel);
	}
#endif//#if QIYIVR_SUPPORTED_PLATFORMS
}

//-----------------------------------------------------------------------------
UQIYIVRHMDEventManager* UQIYIVRHMDFunctionLibrary::GetQIYIVRHMDEventManager()
{
	return UQIYIVRHMDEventManager::GetInstance();
}

#pragma region EyeTracking
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::GetEyeGazeDirection(FVector& OutDirection, bool& Success)
{
	Success = false;

#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();

	if (nullptr == HMD || false == HMD->isEyeTrackingEnabled())
	{
		return;
	}

	Success = HMD->GetRelativeEyeDirection(HMD->HMDDeviceId, OutDirection);

	FQuat HMD_Orientation;
	FVector HMD_Position;

	// rotate eye gaze by HMD rotation
	if (Success &&  (Success = HMD->GetCurrentPose(HMD->HMDDeviceId, HMD_Orientation, HMD_Position) ) )
	{
		OutDirection = HMD_Orientation * OutDirection;

	}


#endif//#if QIYIVR_SUPPORTED_PLATFORMS
	return;
}

//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::GetEyeOrientationAndPosition(FVector& OutPosition, FQuat& OutOrientation, FRotator& OutRotation, bool& Success)
{
	Success = false;
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();

	if (nullptr == HMD || false == HMD->isEyeTrackingEnabled())
	{
		return;
	}

	// doesn't care about eye parameter....
	Success =  HMD->GetRelativeEyePose(HMD->HMDDeviceId, EStereoscopicPass::eSSP_LEFT_EYE, OutOrientation, OutPosition);

	FQuat HMD_Orientation;
	FVector HMD_Position;

	if (Success && (Success = HMD->GetCurrentPose(HMD->HMDDeviceId, HMD_Orientation, HMD_Position)))
	{
		OutPosition = HMD_Orientation * OutPosition + HMD_Position;
		OutOrientation = HMD_Orientation * OutOrientation;
		OutRotation = OutOrientation.Rotator();
	}

#endif//#if QIYIVR_SUPPORTED_PLATFORMS
	return;
}

//-----------------------------------------------------------------------------
// void UQIYIVRHMDFunctionLibrary::GetRelativeEyePose(FVector& OutDirection, FQuat& OutOrientation, bool& Success)
// {
// 	Success = false;
// #if QIYIVR_SUPPORTED_PLATFORMS
// 	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
// 
// 	if (nullptr == HMD || false == HMD->isEyeTrackingEnabled())
// 	{
// 		return;
// 	}
//  unfinished
// 
// 	FVector position, orientation;
// 
// 	if (HMD->GetRelativeEyePose(HMD->HMDDeviceId, EStereoscopicPass.eSSP_LEFT_EYE, OutOrientation, OutPosition);
// 
// 	if ( HMD->GetRelativeEyePose(HMD->HMDDeviceId, EStereoscopicPass.eSSP_LEFT_EYE, OutOrientation, OutPosition);
// 
// #endif//#if QIYIVR_SUPPORTED_PLATFORMS
// 	return;
// }


//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::GetEyeOpenness(EQIYIVRHMD_Eye WhichEye, float& Openness, bool& Success)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
	Success = false;

	if (nullptr == HMD || HMD->isEyeTrackingEnabled())
	{
		return;
	}



	const sxrEyePoseState eps = HMD->GetLatestEyePoseState();

	// Return either eye or averaged value
	Openness = WhichEye == EQIYIVRHMD_Eye::LeftEye ? eps.leftEyeOpenness :
		(WhichEye == EQIYIVRHMD_Eye::RightEye ? eps.rightEyeOpenness : (eps.leftEyeOpenness + eps.rightEyeOpenness)*0.5f);

	Success = true;

#endif//#if QIYIVR_SUPPORTED_PLATFORMS
	return;
}

//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::GetPupilDialation(EQIYIVRHMD_Eye WhichEye, float& Dialation, bool& Success)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
	Success = false;

	if (nullptr == HMD || HMD->isEyeTrackingEnabled())
	{
		return;
	}

	const sxrEyePoseState eps = HMD->GetLatestEyePoseState();

	// Return either eye or averaged value
	Dialation = WhichEye == EQIYIVRHMD_Eye::LeftEye ? eps.leftEyePupilDilation :
		(WhichEye == EQIYIVRHMD_Eye::RightEye ? eps.rightEyePupilDilation : (eps.leftEyePupilDilation + eps.rightEyePupilDilation)*0.5f);

	Success = true;

#endif//#if QIYIVR_SUPPORTED_PLATFORMS
	return;
}

//-----------------------------------------------------------------------------
// void UQIYIVRHMDFunctionLibrary::IsEyetrackingEnabled(bool& IsEnabled)
// {
// #if QIYIVR_SUPPORTED_PLATFORMS
// 	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
// 
// 	IsEnabled = (nullptr != HMD && HMD->isEyeTrackingEnabled()) ? true : false;
// 
// #endif//#if QIYIVR_SUPPORTED_PLATFORMS
// 
// }

#pragma endregion EyeTracking

#pragma region HMD

//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::GetHeadOrientationAndPosition(FVector& OutPosition, FQuat& OutOrientation, FRotator& OutRotator, bool& Success)
{
	Success = false;
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
	if (HMD)
	{
		Success = HMD->GetCurrentPose(HMD->HMDDeviceId, OutOrientation, OutPosition);
		OutRotator = OutOrientation.Rotator();
	}
#endif // QIYIVR_SUPPORTED_PLATFORMS

	return;
}

//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::RecenterOrientation( void )
{
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
	if (HMD)
	{
		HMD->ResetOrientation(0.0f);
	}
#endif // QIYIVR_SUPPORTED_PLATFORMS

	return;
}

//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::RecenterPosition( void )
{
#if QIYIVR_SUPPORTED_PLATFORMS
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
	if (HMD)
	{
		HMD->ResetPosition();
	}
#endif // QIYIVR_SUPPORTED_PLATFORMS

	return;
}

EFLOOREYE_TYPE UQIYIVRHMDFunctionLibrary::GetFloorEyeType()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	return  (EFLOOREYE_TYPE)QiyiVRCore::QVR_GetFloorEyeType();
#endif
	return EFLOOREYE_TYPE::Eye;
}

void  UQIYIVRHMDFunctionLibrary::SetFloorEyeType(EFLOOREYE_TYPE value)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	return  QiyiVRCore::QVR_SetFloorEyeType((int32)value);
#endif
}

/*   parts of the eye struct we've done nothing with yet...
struct sxrEyePoseState
{
	float               leftEyePositionGuide[3];    //!< Position of the inner corner of the left eye in meters from the HMD center-eye coordinate system's origin.
	float               rightEyePositionGuide[3];   //!< Position of the inner corner of the right eye in meters from the HMD center-eye coordinate system's origin.
};
*/

#pragma endregion HMD


#pragma region Foveation
//-----------------------------------------------------------------------------
// Foveation

//-----------------------------------------------------------------------------
bool UQIYIVRHMDFunctionLibrary::isTextureFoveationEnabled()
{
	return GetMutableDefault<UQiyuSettings>()->foveationFeature;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::enableTextureFoveation(bool t)
{
	GetMutableDefault<UQiyuSettings>()->bFoveationEnabled = t;
}

//-----------------------------------------------------------------------------
bool UQIYIVRHMDFunctionLibrary::isTextureFoveationFocusEnabled()
{
	return CVars::TextureFoveationFocusEnabled == 1;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::enableTextureFoveationFocus(bool t)
{
	CVars::TextureFoveationFocusEnabled = t ? 1 : 0;
}


//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::getTextureFoveationGain(float& x, float& y)
{
	x = GetMutableDefault<UQiyuSettings>()->FoveationGain.X;
	y = GetMutableDefault<UQiyuSettings>()->FoveationGain.Y;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::setTextureFoveationGain(float  x, float y)
{
	GetMutableDefault<UQiyuSettings>()->FoveationGain.X = x;
	GetMutableDefault<UQiyuSettings>()->FoveationGain.Y = y;
}


//-----------------------------------------------------------------------------
float UQIYIVRHMDFunctionLibrary::getTextureFoveationArea()
{
	return GetMutableDefault<UQiyuSettings>()->FoveationArea;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::setTextureFoveationArea(float  x)
{
	GetMutableDefault<UQiyuSettings>()->FoveationArea = x;
}
//-----------------------------------------------------------------------------
float UQIYIVRHMDFunctionLibrary::getTextureFoveationFocusAmplitude()
{
	return CVars::TextureFoveationFocusAmplitude;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::setTextureFoveationFocusAmplitude(float  x)
{
	CVars::TextureFoveationFocusAmplitude = x;
}


//-----------------------------------------------------------------------------
float UQIYIVRHMDFunctionLibrary::getTextureFoveationFocusFrequency()
{
	return CVars::TextureFoveationFocusFrequency;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::setTextureFoveationFocusFrequency(float  x)
{
	CVars::TextureFoveationFocusFrequency = x;
}



//-----------------------------------------------------------------------------
//r.SVR.TextureFoveationFocusFrequencyRho
float UQIYIVRHMDFunctionLibrary::getTextureFoveationFocusFrequencyRho()
{
	return CVars::TextureFoveationFocusFrequencyRho;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::setTextureFoveationFocusFrequencyRho(float  x)
{
	CVars::TextureFoveationFocusFrequencyRho = x;
}

#pragma region Eyetracking


//-----------------------------------------------------------------------------
// no set...
void UQIYIVRHMDFunctionLibrary::getAverageGazeDirection(float& x, float& y)
{
	CVars::AverageGazeDirectionX = x;
	CVars::AverageGazeDirectionY = y;
}

//-----------------------------------------------------------------------------
//r.SVR.EyeMarkerRadius
float UQIYIVRHMDFunctionLibrary::getEyetrackingMarkerRadius() 
{
	return CVars::EyeMarkerRadius;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::setEyetrackingMarkerRadius(float  x)
{
	CVars::EyeMarkerRadius = x;
}





//-----------------------------------------------------------------------------
bool UQIYIVRHMDFunctionLibrary::isEyetrackingMarkerEnabled()
{
	return CVars::EyeMarkerEnabled == 1;
}
//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::enableEyetrackingMarker(bool t)
{
	CVars::EyeMarkerEnabled = t ? 1 : 0;
}


//-----------------------------------------------------------------------------
//r.SVR.EyeTrackingEnabled
// no set
bool UQIYIVRHMDFunctionLibrary::isEyetrackingEnabled()
{
	return CVars::EyeTrackingEnabled == 1;
}

#pragma endregion


#if 0 // webhelper

//-----------------------------------------------------------------------------
void UQIYIVRHMDFunctionLibrary::SendMessage(const int32 score)
{
	FQIYIVRHMD* HMD = FQIYIVRHMD::GetQIYIHMD();
	if (HMD)
	{
		HMD->SendMessage(score);
	}
}

#endif // webhelper
