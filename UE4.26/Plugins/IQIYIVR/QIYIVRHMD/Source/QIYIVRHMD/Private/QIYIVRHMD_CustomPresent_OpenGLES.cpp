//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_CustomPresent.h"
#include "QIYIVRHMD_TextureSetProxy.h"
#include "QIYIVR_CVars.h"


#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"

#define dbgprint(format, ...) //FPlatformMisc::LowLevelOutputDebugStringf(*FString::Printf(TEXT("%s line %d: %s"), TEXT(__FILE__), __LINE__, *FString::Printf(TEXT(format), ##__VA_ARGS__)))

extern void GetExtensionsString(FString& ExtensionsString);

#if QIYIVR_SUPPORTED_PLATFORMS
// GL_QCOM_texture_foveated
PFNGLTEXTUREFOVEATIONPARAMETERSQCOMPROC glTextureFoveationParametersQCOM = NULL;
#endif

class FOpenGLESCustomPresentSVR : public FQIYIVRHMDCustomPresent
{
public:
	FOpenGLESCustomPresentSVR(FQIYIVRHMD* pHMD)
		: FQIYIVRHMDCustomPresent(pHMD)
		, mTextureSet(nullptr)
		, mMaxFoveatedFocalPoints(0)
		, ReticleTexture(nullptr)
	{

		CachedFocusPoints[0] = CachedFocusPoints[1] = FVector2D::ZeroVector;
	}

	~FOpenGLESCustomPresentSVR()
	{
		// DARCHARD: TODO -fixme
		//delete mTextureSet;
	}

	void SubmitFrame(const FPoseStateFrame& PoseState) override;

	virtual bool AllocateRenderTargetTexture(
		uint32 SizeX,
		uint32 SizeY,
		uint8 Format,
		uint32 NumMips, 
		uint32 Flags,
		uint32 TargetableTextureFlags,
		FTexture2DRHIRef& OutTargetableTexture,
		FTexture2DRHIRef& OutShaderResourceTexture,
		uint32 NumSamples) override;

private:
	class FQIYIVRTextureSet_OpenGL* mTextureSet;

	uint32 mMaxFoveatedFocalPoints;

	FTexture2DRHIRef ReticleTexture;

	// If eye-tracking is enabled and no valid data is available for either eye this frame, fall back to the last known gaze points
	FVector2D CachedFocusPoints[2];
};

FQIYIVRHMDCustomPresent* CreateCustomPresent_OpenGLES(class FQIYIVRHMD* pHMD)
{
	return new FOpenGLESCustomPresentSVR(pHMD);
}

bool FOpenGLESCustomPresentSVR::AllocateRenderTargetTexture(
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
#if QIYIVR_SUPPORTED_PLATFORMS
	FOpenGLDynamicRHI* OpenGLRHI = static_cast<FOpenGLDynamicRHI*>(GDynamicRHI);

	//FDebug::DumpStackTraceToLog(); // RBF
	dbgprint("FOpenGLESCustomPresentSVR::AllocateRenderTargetTexture called");

	FString ExtensionsString;
	GetExtensionsString(ExtensionsString);

	bool bSupportsTextureBasedFoveation = ExtensionsString.Contains(TEXT("GL_QCOM_texture_foveated"));
	if (bSupportsTextureBasedFoveation)
	{
		dbgprint("GL_QCOM_texture_foveated: Extension string is exposed");

		// CTORNE: This is global state, it doesn't matter what texture target we pass in and whether an actual texture is bound.
		// The extension should be changed to rely on the global glGetIntegerv query instead.
		GLint QueryResult = 0;
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM, &QueryResult);

		checkf((QueryResult & GL_FOVEATION_ENABLE_BIT_QCOM) == GL_FOVEATION_ENABLE_BIT_QCOM &&
			   (QueryResult & GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM) == GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM,
			   TEXT("GL_QCOM_texture_foveated: The implementation does not support the scaled bin foveation method despite exposing the extension!"));
		
		dbgprint("GL_QCOM_texture_foveated: Scaled bin foveation method supported");

		float DefaultMinFoveatedPixelDensity = -1.f;
		glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM, &DefaultMinFoveatedPixelDensity);

		checkf(DefaultMinFoveatedPixelDensity >= 0.f && DefaultMinFoveatedPixelDensity <= 1.f,
			   TEXT("GL_QCOM_texture_foveated: The minimum pixel density supported (%f), should be [0.f;1.f]!"), DefaultMinFoveatedPixelDensity);

		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM, &QueryResult);

		checkf(QueryResult > 0,
			   TEXT("GL_QCOM_texture_foveated: The Maximum number of focal points per texture layer (%d), should be >= 1!"), mMaxFoveatedFocalPoints);
		
		mMaxFoveatedFocalPoints = static_cast<GLuint>(QueryResult);
		dbgprint("GL_QCOM_texture_foveated: Maximum number of focal points per texture layer = %u", mMaxFoveatedFocalPoints);

		glTextureFoveationParametersQCOM = (PFNGLTEXTUREFOVEATIONPARAMETERSQCOMPROC)((void*)eglGetProcAddress("glTextureFoveationParametersQCOM"));

		checkf(glTextureFoveationParametersQCOM != nullptr,
			   TEXT("Couldn't find entry point glTextureFoveationParametersQCOM!"));
	}
	else
	{
		CVars::TextureFoveationEnabled = 0;
	}

	static const auto MobileMultiViewCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
	static const bool bIsUsingMobileMultiView = GSupportsMobileMultiView && MobileMultiViewCVar && MobileMultiViewCVar->GetValueOnAnyThread() != 0;

	static const auto MobileMultiViewDirectCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView.Direct"));
	static const bool bIsUsingMobileMultiViewDirect = bIsUsingMobileMultiView && MobileMultiViewDirectCVar && MobileMultiViewDirectCVar->GetValueOnAnyThread() != 0;

	GLenum Target = GL_TEXTURE_2D;
	uint32 Layers = 1;
	if (bIsUsingMobileMultiViewDirect)
	{
		Target = GL_TEXTURE_2D_ARRAY;
		Layers = 2;
		bUsesArrayTexture = true;
	}

	uint32 Width = SizeX / Layers;

	mTextureSet = CreateTextureSetProxy_OpenGLES(
		OpenGLRHI,
		Target,
		Width,
		SizeY,
		Layers,
		Format,
		Flags,
		TargetableTextureFlags,
		OutTargetableTexture,
		OutShaderResourceTexture);
#endif
	return true;
}

extern GLuint TextureSetProxy_OpenGLES_GetNativeResource(class FQIYIVRTextureSet_OpenGL* TextureSetProxy);
extern void TextureSetProxy_SwitchToNextElement(FQIYIVRTextureSet_OpenGL* TextureSetProxy);

void FOpenGLESCustomPresentSVR::SubmitFrame(const FPoseStateFrame& PoseState)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	sxrFrameParams FrameParams;
	memset(&FrameParams, 0, sizeof(FrameParams));
	FrameParams.frameIndex = (int32)PoseState.FrameNumber;
	GLuint TextureHandle = TextureSetProxy_OpenGLES_GetNativeResource(mTextureSet);
	// UE_LOG(LogSVR, Log, TEXT(" FOpenGLESCustomPresentSVR::SubmitFrame - pose is Position = x:%5.2f, y:%5.2f, z:%5.2f    Rotation=[%5.3f,%5.3f,%5.3f,%5.3f]"),
	// 	PoseState.Pose.pose.position.x, PoseState.Pose.pose.position.y, PoseState.Pose.pose.position.z,
	// 	PoseState.Pose.pose.rotation.x, PoseState.Pose.pose.rotation.y, PoseState.Pose.pose.rotation.z, PoseState.Pose.pose.rotation.w
	// );
		/// RBF TODO
	if (UsesArrayTexture())
	{
		// Left Eye
		FrameParams.renderLayers[0].imageType = kTypeTextureArray;
		FrameParams.renderLayers[0].imageHandle = TextureHandle;
		CreateLayout(0.0f, 0.0f, 1.0f, 1.0f, &FrameParams.renderLayers[0].imageCoords);
		FrameParams.renderLayers[0].eyeMask = kEyeMaskLeft;
		FrameParams.renderLayers[0].layerFlags |= kLayerFlagOpaque;

		// Right Eye
		FrameParams.renderLayers[1].imageType = kTypeTextureArray;
		FrameParams.renderLayers[1].imageHandle = TextureHandle;
		CreateLayout(0.0f, 0.0f, 1.0f, 1.0f, &FrameParams.renderLayers[1].imageCoords);
		FrameParams.renderLayers[1].eyeMask = kEyeMaskRight;
		FrameParams.renderLayers[1].layerFlags |= kLayerFlagOpaque;
		// UE_LOG(LogSVR, Log, TEXT("FOpenGLESCustomPresentSVR::SubmitFrame - rendering texture ID %d - from a texture array"), TextureHandle);

	}
	else
	{
		FrameParams.renderLayers[0].imageHandle = TextureHandle;
		FrameParams.renderLayers[0].imageType = kTypeTexture;
		CreateLayout(0.0f, 0.0f, 1.0f, 1.0f, &FrameParams.renderLayers[0].imageCoords);
		FrameParams.renderLayers[0].imageCoords.LowerUVs[2] = 0.5f;
		FrameParams.renderLayers[0].imageCoords.UpperUVs[2] = 0.5f;
		FrameParams.renderLayers[0].eyeMask = kEyeMaskLeft;
		FrameParams.renderLayers[0].layerFlags |= kLayerFlagOpaque;

		FrameParams.renderLayers[1].imageHandle = TextureHandle;
		FrameParams.renderLayers[1].imageType = kTypeTexture;
		CreateLayout(0.0f, 0.0f, 1.0f, 1.0f, &FrameParams.renderLayers[1].imageCoords);
		FrameParams.renderLayers[1].imageCoords.LowerUVs[0] = 0.5f;
		FrameParams.renderLayers[1].imageCoords.UpperUVs[0] = 0.5f;
		FrameParams.renderLayers[1].eyeMask = kEyeMaskRight;
		FrameParams.renderLayers[1].layerFlags |= kLayerFlagOpaque;
		// UE_LOG(LogSVR, Log, TEXT("FOpenGLESCustomPresentSVR::SubmitFrame - not using array"));
	}

	// Focal point is provided in normalized device coordinates, (-1,-1) is the lower left and (+1,+1) is the upper right corner of our render target
	FVector2D FocusPoints[2] = { CachedFocusPoints[0], CachedFocusPoints[1] };

	// Only update the focus point if required (foveation is enabled and/or we're drawing the eye marker)
	if (CVars::TextureFoveationEnabled != 0 || CVars::EyeMarkerEnabled != 0)
	{
		if (CVars::EyeTrackingEnabled != 0)
		{
			sxrEyePoseState EyePose;
			memset(&EyePose, 0, sizeof(EyePose));

			if ( sxrGetEyePose(&EyePose) != SXR_ERROR_NONE )
			{
				// Yes yes, but if you take the HMD off it crashes the service and you need to reboot the device
				// so, F that
				goto skipFoveationAndEyeTracking;
			}

			dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - eye pose");


			if (CVars::LogEyePoseData)
			{
#define AppendBitFieldName(BitField, Name) \
				if ((status & BitField) == BitField)\
				{\
					status &= ~BitField;\
					if (String.Len() > 0)\
					{\
						String += TEXT("|");\
					}\
					String += Name;\
				}

				auto GetEyeStatusString = [](int32_t status)->FString
				{
					if (status == 0)
					{
						return TEXT("NO VALID DATA");
					}
					FString String;
					AppendBitFieldName(kGazePointValid, TEXT("GazePointValid"));
					AppendBitFieldName(kGazeVectorValid, TEXT("GazeVectorValid"));
					AppendBitFieldName(kEyeOpennessValid, TEXT("EyeOpennessValid"));
					AppendBitFieldName(kEyePupilDilationValid, TEXT("EyePupilDilationValid"));
					AppendBitFieldName(kEyePositionGuideValid, TEXT("EyePositionGuideValid"));
					if (status != 0)
					{
						FString Unknown = FString::Printf(TEXT("Unknown (%d)"), status);
						AppendBitFieldName(status, Unknown);
					}
					return String;
				};

#undef AppendBitFieldName

				dbgprint(
					"Frame %llu, EyePose:\n"
					"	leftEyePoseStatus = %d (%s), rightEyePoseStatus = %d (%s), combinedEyePoseStatus = %d (%s)\n"
					"	leftEyeGazeVector = (%f, %f, %f), rightEyeGazeVector = (%f, %f, %f), combinedEyeGazeVector = (%f, %f, %f)\n",
					GFrameNumber,
					EyePose.leftEyePoseStatus, *GetEyeStatusString(EyePose.leftEyePoseStatus),
					EyePose.rightEyePoseStatus, *GetEyeStatusString(EyePose.rightEyePoseStatus),
					EyePose.combinedEyePoseStatus, *GetEyeStatusString(EyePose.combinedEyePoseStatus),
					EyePose.leftEyeGazeVector[0], EyePose.leftEyeGazeVector[1], EyePose.leftEyeGazeVector[2],
					EyePose.rightEyeGazeVector[0], EyePose.rightEyeGazeVector[1], EyePose.rightEyeGazeVector[2],
					EyePose.combinedEyeGazeVector[0], EyePose.combinedEyeGazeVector[1], EyePose.combinedEyeGazeVector[2]
				);
			}

			FVector EyeDirections[3] =
			{
				FVector(EyePose.leftEyeGazeVector[0], EyePose.leftEyeGazeVector[1], -EyePose.leftEyeGazeVector[2]),
				FVector(EyePose.rightEyeGazeVector[0], EyePose.rightEyeGazeVector[1], -EyePose.rightEyeGazeVector[2]),
				FVector(EyePose.combinedEyeGazeVector[0], EyePose.combinedEyeGazeVector[1], -EyePose.combinedEyeGazeVector[2])
			};

			bool bUsesCombinedGazeDirection = (CVars::AverageGazeDirectionX == CVars::kUseCombinedGazeData || CVars::AverageGazeDirectionY == CVars::kUseCombinedGazeData);
			bool bUsesIndividualGazeDirections = !(CVars::AverageGazeDirectionX == CVars::kUseCombinedGazeData && CVars::AverageGazeDirectionY == CVars::kUseCombinedGazeData);

			bool bGazeVectorValid[3] =
			{
				(EyePose.leftEyePoseStatus & kGazeVectorValid) != 0,
				(EyePose.rightEyePoseStatus & kGazeVectorValid) != 0,
				(EyePose.combinedEyePoseStatus & kGazeVectorValid) != 0
			};

			bool bUseCachedFocalPoints = false;

			if (bUsesCombinedGazeDirection && !bGazeVectorValid[2])
			{
				if (CVars::LogEyePoseData)
				{
					UE_LOG(LogSVR, Warning, TEXT("Combined eye gaze vector data is invalid."));
				}
				bUseCachedFocalPoints = true;
				dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - bUsesCombinedGazeDirection");
			}

			if (bUsesIndividualGazeDirections)
			{
				dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 1");
				if (!bGazeVectorValid[0])
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 1a");
					if (CVars::LogEyePoseData)
					{
						UE_LOG(LogSVR, Warning, TEXT("Left eye gaze vector data is invalid."));
					}
					bUseCachedFocalPoints = true;
				}

				if (!bGazeVectorValid[1])
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 2");
					if (CVars::LogEyePoseData)
					{
						UE_LOG(LogSVR, Warning, TEXT("Right eye gaze vector data is invalid."));
					}
					bUseCachedFocalPoints = true;
				}
			}			

			if (!bUseCachedFocalPoints)
			{
				dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 3");
				if (CVars::AverageGazeDirectionX == CVars::kUseCombinedGazeData)
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 3a");
					EyeDirections[0].X = EyeDirections[1].X = EyeDirections[2].X;
				}
				else if (CVars::AverageGazeDirectionX == CVars::kComputeAverageGaze)
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 3b");
					EyeDirections[0].X = EyeDirections[1].X = 0.5f * (EyeDirections[0].X + EyeDirections[1].X);
				}

				if (CVars::AverageGazeDirectionY == CVars::kUseCombinedGazeData)
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 3c");
					EyeDirections[0].Y = EyeDirections[1].Y = EyeDirections[2].Y;
				}
				else if (CVars::AverageGazeDirectionY == CVars::kComputeAverageGaze)
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 3d");
					EyeDirections[0].Y = EyeDirections[1].Y = 0.5f * (EyeDirections[0].Y + EyeDirections[1].Y);
				}

				sxrDeviceInfo DeviceInfo = sxrGetDeviceInfo();

				float NearPlanesDistances[2] = { DeviceInfo.leftEyeFrustum.near, DeviceInfo.rightEyeFrustum.near };

				FVector4 NearPlaneExtents[2] =
				{
					FVector4(DeviceInfo.leftEyeFrustum.left, DeviceInfo.leftEyeFrustum.right, DeviceInfo.leftEyeFrustum.bottom, DeviceInfo.leftEyeFrustum.top),
					FVector4(DeviceInfo.rightEyeFrustum.left, DeviceInfo.rightEyeFrustum.right, DeviceInfo.rightEyeFrustum.bottom, DeviceInfo.rightEyeFrustum.top)
				};

				for (uint32 EyeIndex = 0; EyeIndex < 2; EyeIndex++)
				{
					dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 4");
					if (!FMath::IsNearlyZero(EyeDirections[EyeIndex].Z) && bGazeVectorValid[EyeIndex])
					{
						dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 4a");

						// Determine the intersection of the gaze direction with the near plane using similar triangles:
						//
						//   GazeDir.Z   GazeDir.X   GazeDir.Y
						//   --------- = --------- = ---------
						//      Near         X           Y    
						//
						//   Where Near is the distance to the near plane, i.e. its Z coordinate, and (X,Y) is the intersection point on the near plane
						FVector2D Intersection;
						Intersection.X = (NearPlanesDistances[EyeIndex] * EyeDirections[EyeIndex].X) / EyeDirections[EyeIndex].Z;
						Intersection.Y = (NearPlanesDistances[EyeIndex] * EyeDirections[EyeIndex].Y) / EyeDirections[EyeIndex].Z;

						// Now we need to remap the intersection X coordinate from [Left;Right] and the Y coordinate from [Bottom;Top] to [-1;+1]
						// Note that our eye frusta can be asymmetrical, so left != (-right) necessarily (same for bottom and top).

						// X in [L,R] -> x in [-1,1] => x == -1 + (X-L)*(1-(-1))/(R-L)
						check(NearPlaneExtents[EyeIndex].X != NearPlaneExtents[EyeIndex].Y);
						FocusPoints[EyeIndex].X = -1.f + 2.f*(Intersection.X - NearPlaneExtents[EyeIndex].X) / (NearPlaneExtents[EyeIndex].Y - NearPlaneExtents[EyeIndex].X);

						// Y in [B,T] -> y in [-1,1] => y == -1 + (Y-B)*(1-(-1))/(T-B)
						check(NearPlaneExtents[EyeIndex].Z != NearPlaneExtents[EyeIndex].W);
						FocusPoints[EyeIndex].Y = -1.f + 2.f*(Intersection.Y - NearPlaneExtents[EyeIndex].Z) / (NearPlaneExtents[EyeIndex].W - NearPlaneExtents[EyeIndex].Z);

						CachedFocusPoints[EyeIndex] = FocusPoints[EyeIndex];
					}
				}
			}
		} // 		if (CVars::EyeTrackingEnabled != 0)
		else if (CVars::TextureFoveationFocusEnabled != 0)
		{
			dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 5");
			float Time = GRenderingRealtimeClock.GetCurrentTime();
			float Radius = CVars::TextureFoveationFocusAmplitude * FMath::Abs(FMath::Cos(Time * CVars::TextureFoveationFocusFrequencyRho));

			FocusPoints[0].X = Radius * FMath::Cos(Time * CVars::TextureFoveationFocusFrequency);
			FocusPoints[0].Y = Radius * FMath::Sin(Time * CVars::TextureFoveationFocusFrequency);

			// They're all the same point
			CachedFocusPoints[0] = CachedFocusPoints[1] = FocusPoints[1] = FocusPoints[0];
		}
	} // 	if (TextureFoveationEnabled != 0 || CVars::EyeMarkerEnabled != 0)

	if (CVars::TextureFoveationEnabled != 0)
	{
		if (UsesArrayTexture())
		{
			dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 6");
			glTextureFoveationParametersQCOM(TextureHandle,         // texture
											 0,						// layer
											 0,						// focalPoint
											 FocusPoints[0].X,		// focalX
											 FocusPoints[0].Y,		// focalY
											 CVars::TextureFoveationGainX, // gainX
											 CVars::TextureFoveationGainY,	// gainY
											 CVars::TextureFoveationArea	// foveaArea
			);

			glTextureFoveationParametersQCOM(TextureHandle,         // texture
											 1,						// layer
											 0,						// focalPoint
											 FocusPoints[1].X,		// focalX
											 FocusPoints[1].Y,		// focalY
										CVars::TextureFoveationGainX, // gainX
										CVars::TextureFoveationGainY,	// gainY
										CVars::TextureFoveationArea	// foveaArea
			);
		}
		else
		{
			dbgprint("FOpenGLESCustomPresentSVR::SubmitFrame - 6a");
			// Make sure we support specifying 2 focal points
			checkf(mMaxFoveatedFocalPoints >= 2, TEXT("GL_QCOM_texture_foveated: Need 2 focal points for double-wide render target!"));
			
			// In our case, the focal points will be (-0.5, 0.0) and (+0.5, 0.0), respectively. X-axis gain should twice Y-axis gain to compensate for the width
			glTextureFoveationParametersQCOM(TextureHandle,						// texture
											 0,									// layer
											 0,									// focalPoint
											 -0.5f + FocusPoints[0].X * 0.5f,	// focalX
											 FocusPoints[0].Y,					// focalY
											 CVars::TextureFoveationGainX * 2.f,		// gainX
											 CVars::TextureFoveationGainY,				// gainY
											 CVars::TextureFoveationArea		// foveaArea
			);

			glTextureFoveationParametersQCOM(TextureHandle,						// texture
											 0,									// layer
											 1,									// focalPoint
											 0.5f + FocusPoints[1].X * 0.5f,	// focalX
											 FocusPoints[1].Y,					// focalY
											 CVars::TextureFoveationGainX * 2.f,		// gainX
											 CVars::TextureFoveationGainY,				// gainY
											 CVars::TextureFoveationArea		// foveaArea
			);
		}
	}

	skipFoveationAndEyeTracking:

	FrameParams.headPoseState = PoseState.Pose;
	FrameParams.minVsyncs = 1;

	// sxrprint("(%s) (Frame %d) SubmitFrame(mRenderPose) => Calling sxrSubmitFrame(Frame %d)", IsInRenderingThread() ? TEXT("Render") : TEXT("Game"), GFrameNumber, FrameParams.frameIndex);

	sxrSubmitFrame(&FrameParams);
	TextureSetProxy_SwitchToNextElement(mTextureSet);

#endif
}

