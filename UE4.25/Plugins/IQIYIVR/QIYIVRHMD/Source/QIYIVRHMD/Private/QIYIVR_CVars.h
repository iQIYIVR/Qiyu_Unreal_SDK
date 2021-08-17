//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#pragma once
#include "Math/Vector.h"
#include "Math/Quat.h"


#define CVR_EXTRN(TYPE,NAME,INIT_VAL) extern TYPE NAME
#define DEFINE_CVAR(TYPE,NAME,INIT_VAL)  TYPE NAME = INIT_VAL


namespace CVars
{

#ifndef INCLUDE_CVARS
#define INCLUDE_CVARS
	enum AverageGazeMode
	{
		kNoAverage = 0,
		kUseCombinedGazeData,
		kComputeAverageGaze
		};
#endif


	// the variables
	CVR_EXTRN(int32, TextureFoveationFocusAmplitude, 0);
	CVR_EXTRN(int32, TextureFoveationFocusEnabled, 0);
	CVR_EXTRN(float, TextureFoveationFocusFrequency, 0.0f);
	CVR_EXTRN(float, TextureFoveationFocusFrequencyRho, 0.0f);



	CVR_EXTRN(int32, EyeTrackingEnabled, 1);
	CVR_EXTRN(int32, AverageGazeDirectionX, 0);
	CVR_EXTRN(int32, AverageGazeDirectionY, 0);
	CVR_EXTRN(int32, EyeMarkerEnabled, 0);
	CVR_EXTRN(float, EyeMarkerRadius, 0.0f);
	CVR_EXTRN(int32, LogEyePoseData, 0);

	CVR_EXTRN(int32, GNumSwapchainImages, 3);

	//CVR_EXTRN(FString, SXRViewFrustumLeft_string, "0.1 0.2 0.3 0.4 0.5 0.6 0.7"); // position (3 floats) then quaternion (4 floats)
}




