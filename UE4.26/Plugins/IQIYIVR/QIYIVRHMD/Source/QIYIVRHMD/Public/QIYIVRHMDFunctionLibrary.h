//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "../Classes/QIYIVRHMDEventManager.h"
//#include "QIYIVRHMD.h"
#include "IQIYIVRHMDModule.h"
#include "QiyiVRSDKCore.h"
#include "QIYIVRHMDFunctionLibrary.generated.h"


extern int32 EyeTrackingEnabled;


UENUM(BlueprintType)
enum class EQIYIVRHMD_Eye : uint8
{
	LeftEye = 0,
	RightEye,
	AverageEye
};

 UENUM(BlueprintType)
 enum class EFLOOREYE_TYPE : uint8
 {
 	Eye = 0,
 	Floor = 1,
 };

UCLASS()
class UQIYIVRHMDFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR"))
		static void SetCpuAndGpuLevelsSVR(const int32 CpuLevel, const int32 GpuLevel);

	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR"))
		static UQIYIVRHMDEventManager* GetQIYIVRHMDEventManager();

#pragma region Eyetracking
	// Eye Tracking

	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR Eyetracking", Tooltip = "Gaze Direction in World Coordinates"))
		static void GetEyeGazeDirection(FVector& OutDirection, bool& Success);
	
	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR Eyetracking", Tooltip = "Direction and Orientation in World Coordinates"))
		static void GetEyeOrientationAndPosition(FVector& OutPosition, FQuat& OutOrientation, FRotator& OutRotation, bool& Success);

//	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR Eyetracking", Tooltip = "Direction and Orientation of current pose in World coordinate system"))
//		static void GetRelativeEyePose(FVector& OutPosition, FQuat& OutOrientation, bool& Success);

	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR Eyetracking"))
		static void GetEyeOpenness(EQIYIVRHMD_Eye WhichEye, float& Openness, bool& Success);

	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR Eyetracking"))
		static void GetPupilDialation(EQIYIVRHMD_Eye WhichEye, float& Openness, bool& Success);

//	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR Eyetracking"))
//		static void IsEyetrackingEnabled(bool& IsEnabled);

#pragma endregion EyeTracking

#pragma region HMD
	// HMD 

	UFUNCTION(BlueprintPure, Category = "QIYIVR", meta = (Keywords = "QIYIVR HMD", Tooltip = "Direction and Orientation/Rotator of current head pose in World coordinate system"))
		static void GetHeadOrientationAndPosition(FVector& OutPosition, FQuat& OutOrientation, FRotator& OutRotator, bool& Success);

//	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR HMD", Tooltip = "Reset the HDM Orientation, AroundVerticalAxis = true then just around the verical axis (no tilt)"))
	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR HMD", Tooltip = "Recenter the HDM's Orientation"))
		static void RecenterOrientation( );

	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR HMD", Tooltip = "Recenter the HMD's position"))
		static void RecenterPosition( );

 	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR HMD", Tooltip = "GetFloorEyeType"))
 		static EFLOOREYE_TYPE GetFloorEyeType();
 
 	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR HMD", Tooltip = "SetFloorEyeType"))
 		static void  SetFloorEyeType(EFLOOREYE_TYPE value);

#pragma endregion HMD

#pragma endregion Foveation
	// FOVEATION

	UFUNCTION(BlueprintPure, Category = "Temporal|Foveation|QIYIVR", meta = (DisplayName = "is Temporal Foveation Enabled?", Keywords = "Foveation"))
		static bool isTemporalFoveationEnabled();

	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "is Texture Foveation Enabled?", Keywords = "Foveation"))
		static bool isTextureFoveationEnabled();

	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "is Texture Foveation Focus Enabled?", Keywords = "Foveation"))
		static bool isTextureFoveationFocusEnabled();


	UFUNCTION(BlueprintCallable, Category = "Temporal|Foveation|QIYIVR", meta = (DisplayName = "enable Temporal Foveation", Keywords = "Foveation"))
		static void enableTemporalFoveation(bool t);

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "enable Texture Foveation", Keywords = "Foveation"))
		static void enableTextureFoveation(bool t);

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "enable Texture Foveation Focus", Keywords = "Foveation"))
		static void enableTextureFoveationFocus(bool t);


	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "get Texture Foveation Gain", Keywords = "Foveation"))
		static void getTextureFoveationGain(float& x, float& y);


	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "set Texture Foveation Gain", Keywords = "Foveation"))
		static void setTextureFoveationGain(float  x, float y);

	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "get Texture Foveation Area", Keywords = "Foveation"))
		static float getTextureFoveationArea();

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "set Texture Foveation Area", Keywords = "Foveation"))
		static void setTextureFoveationArea(float  x);

	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "get Texture Minimum Pixel Density", Keywords = "Foveation"))
		static float getTextureMinimumPixelDensity();

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "set Texture Minimum Pixel Density", Keywords = "Foveation"))
		static void setTextureMinimumPixelDensity(float  x);



	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "get Texture Foveation Focus Amplitude", Keywords = "Foveation"))
		static float getTextureFoveationFocusAmplitude();

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "set Texture Foveation Focus Amplitude", Keywords = "Foveation"))
		static void setTextureFoveationFocusAmplitude(float  x);

	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "get Texture Foveation Focus Frequency", Keywords = "Foveation"))
		static float getTextureFoveationFocusFrequency();

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "set Texture Foveation Focus Frequency", Keywords = "Foveation"))
		static void setTextureFoveationFocusFrequency(float  x);

	UFUNCTION(BlueprintPure, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "get Texture Foveation Focus Frequency Rho", Keywords = "Foveation"))
		static float getTextureFoveationFocusFrequencyRho();

	UFUNCTION(BlueprintCallable, Category = "Texture|Foveation|QIYIVR", meta = (DisplayName = "set Texture Foveation Focus Frequency Rho", Keywords = "Foveation"))
		static void setTextureFoveationFocusFrequencyRho(float  Rho);

#pragma endregion HMD

	// eyetracking

	UFUNCTION(BlueprintPure, Category = "Eyetracking|QIYIVR", meta = (DisplayName = "get average gaze direction", Keywords = "Eyetracking"))
		static void getAverageGazeDirection(float& x, float& y);

	UFUNCTION(BlueprintPure, Category = "Eyetracking|QIYIVR", meta = (DisplayName = "get Texture Eyetracking Marker Radius", Keywords = "Eyetracking"))
		static float getEyetrackingMarkerRadius();

	UFUNCTION(BlueprintCallable, Category = "Eyetracking|QIYIVR", meta = (DisplayName = "set Texture Eyetracking Marker Radius", Keywords = "Eyetracking"))
		static void setEyetrackingMarkerRadius(float  x);

	UFUNCTION(BlueprintPure, Category = "Eyetracking|QIYIVR", meta = (DisplayName = "is EyetrackingMarker enabled", Keywords = "Eyetracking"))
		static bool isEyetrackingMarkerEnabled();

	UFUNCTION(BlueprintCallable, Category = "Eyetracking|QIYIVR", meta = (DisplayName = "enable Eyetracking Marker?", Keywords = "Eyetracking"))
		static void enableEyetrackingMarker(bool v);

	UFUNCTION(BlueprintPure, Category = "Eyetracking|QIYIVR", meta = (DisplayName = "is Eyetracking enabled", Keywords = "Eyetracking"))
		static bool isEyetrackingEnabled();

// Webhelper
//	UFUNCTION(BlueprintCallable, Category = "QIYIVR", meta = (Keywords = "QIYIVR"))
//		static void SendMessage(const int32 score);
};
