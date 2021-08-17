//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "QIYIVRHMD_TextureSetProxy.h"

#include "OpenGLDrvPrivate.h"
#include "OpenGLResources.h"
#include "QIYIVR_CVars.h"
#include "QIYIVRHMDPrivateRHI.h"



// move this somewhere - rbf
#define dbgprint(format, ...) //FPlatformMisc::LowLevelOutputDebugStringf(*FString::Printf(TEXT("%s line %d: %s"), TEXT(__FILE__), __LINE__, *FString::Printf(TEXT(format), ##__VA_ARGS__)))

#define GL_TEXTURE_PREVIOUS_SOURCE_TEXTURE_QCOM         0x8BE8
class FQIYIVRTextureSet_OpenGL : public FOpenGLTexture2D, public FQIYIVRTextureSet
{
public:
	FQIYIVRTextureSet_OpenGL(
		class FOpenGLDynamicRHI* InRHI,
		GLenum InTarget,
		uint32 InSizeX,
		uint32 InSizeY,
		uint32 InArraySize,
		uint8  InFormat,
		uint32 InFlags,
		uint32 InTargetableTexureFlags);

	virtual ~FQIYIVRTextureSet_OpenGL() override;

protected:
	virtual void SetNativeResource(const FTextureRHIRef& RenderTargetTexture) override;

private:
	FQIYIVRTextureSet_OpenGL(const FQIYIVRTextureSet_OpenGL &) = delete;
	FQIYIVRTextureSet_OpenGL(FQIYIVRTextureSet_OpenGL &&) = delete;
	FQIYIVRTextureSet_OpenGL &operator=(const FQIYIVRTextureSet_OpenGL &) = delete;
};

FQIYIVRTextureSet_OpenGL::~FQIYIVRTextureSet_OpenGL() {}

FQIYIVRTextureSet_OpenGL::FQIYIVRTextureSet_OpenGL(
	class FOpenGLDynamicRHI* InGLRHI,
	GLenum InTarget,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InArraySize,
	uint8  InFormat,
	uint32 InFlags,
	uint32 InTargetableTextureFlags
) : FOpenGLTexture2D(
	InGLRHI,
	0,
	InTarget,
	0,
	InSizeX,
	InSizeY,
	0, // UE4 expects SizeZ=0 instead of 1 for anything but 3D textures
	1,
	1,
	1,
	InArraySize,
	(EPixelFormat)InFormat,
	false,
	false,
#if ENGINE_MINOR_VERSION  > 25	
	(ETextureCreateFlags)InFlags,
#else
	InFlags,
#endif
#if ENGINE_MINOR_VERSION == 25	
	nullptr,
#endif
	FClearValueBinding::Black)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	check(InArraySize == 1 || InTarget == GL_TEXTURE_2D_ARRAY);

	// rbf debug
	dbgprint("FQIYIVRTextureSet_OpenGL c'tor - start");


	for (int i = 0; i < CVars::GNumSwapchainImages; ++i)
	{
		bool bArrayTexture = (InArraySize > 1);

		bool bNoSRGBSupport = (GMaxRHIFeatureLevel == ERHIFeatureLevel::ES3_1);

		if ((InTargetableTextureFlags & TexCreate_RenderTargetable) && InFormat == PF_B8G8R8A8 
#if ENGINE_MINOR_VERSION == 25	
			&& !FOpenGL::SupportsBGRA8888RenderTarget()
#endif
			)
		{
			// Some android devices does not support BGRA as a color attachment
			InFormat = PF_R8G8B8A8;
		}

		if (bNoSRGBSupport)
		{
			// Remove sRGB read flag when not supported
			InTargetableTextureFlags &= ~TexCreate_SRGB;
		}

		GLuint TextureID = 0;
		FOpenGL::GenTextures(1, &TextureID);

		// rbf debug
		dbgprint("FQIYIVRTextureSet_OpenGL c'tor - creating texture ID %d, Size %d,%d format - %d",
			TextureID, InSizeX, InSizeY, (uint)InFormat);

		const bool bSRGB = (InTargetableTextureFlags & TexCreate_SRGB) != 0;
		const FOpenGLTextureFormat& GLFormat = GOpenGLTextureFormats[InFormat];
		if (GLFormat.InternalFormat[bSRGB] == GL_NONE)
		{
			UE_LOG(LogRHI, Fatal, TEXT("Texture format '%s' not supported (sRGB=%d)."), GPixelFormats[InFormat].Name, bSRGB);
		}

		FOpenGLContextState& ContextState = InGLRHI->GetContextStateForCurrentContext();

		// Make sure PBO is disabled
		InGLRHI->CachedBindPixelUnpackBuffer(ContextState, 0);

		// Use a texture stage that's not likely to be used for draws, to avoid waiting
		InGLRHI->CachedSetupTextureStage(ContextState, FOpenGL::GetMaxCombinedTextureImageUnits() - 1, InTarget, TextureID, 0, 1);

		if (!FMath::IsPowerOfTwo(InSizeX) || !FMath::IsPowerOfTwo(InSizeY))
		{
			glTexParameteri(InTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(InTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(InTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(InTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexParameteri(InTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(InTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		if (FOpenGL::SupportsTextureFilterAnisotropic())
		{
			glTexParameteri(InTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
		}

		//if (FOpenGL::SupportsTextureBaseLevel())
		{
			glTexParameteri(InTarget, GL_TEXTURE_BASE_LEVEL, 0);
		}

		//if (FOpenGL::SupportsTextureMaxLevel())
		{
			glTexParameteri(InTarget, GL_TEXTURE_MAX_LEVEL, 0);
		}

		// TODO: Figure out if this is required
		//InGLRHI->TextureMipLimits.Add(TextureID, TPair<GLenum, GLenum>(0, 0));

		if (/*FOpenGL::SupportsTextureSwizzle()*/ false && GLFormat.bBGRA && !(InTargetableTextureFlags & TexCreate_RenderTargetable))
		{
			glTexParameteri(InTarget, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
			glTexParameteri(InTarget, GL_TEXTURE_SWIZZLE_B, GL_RED);
		}

		if (CVars::TextureFoveationEnabled != 0)
		{
			glTexParameteri(InTarget, GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM, GL_FOVEATION_ENABLE_BIT_QCOM | GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM);
			
			dbgprint("GL_QCOM_texture_foveated: Enabling scaled bin foveation on texture %u", TextureID);

			checkf(CVars::TextureFoveationMinPixelDensity >= 0.f && CVars::TextureFoveationMinPixelDensity <= 1.f,
				TEXT("GL_QCOM_texture_foveated: The minimum pixel density (%f), should be [0.f;1.f]!"), CVars::TextureFoveationMinPixelDensity);

			glTexParameterf(InTarget, GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM, CVars::TextureFoveationMinPixelDensity);

			dbgprint("GL_QCOM_texture_foveated: Minimum pixel density = %f", CVars::TextureFoveationMinPixelDensity);
		}

		if (bArrayTexture)
		{
			FOpenGL::TexStorage3D(InTarget, 1, GLFormat.InternalFormat[bSRGB], InSizeX, InSizeY, InArraySize, GLFormat.Format, GLFormat.Type);
		}
		else if (!FOpenGL::TexStorage2D(InTarget, 1, GLFormat.SizedInternalFormat[bSRGB], InSizeX, InSizeY, GLFormat.Format, GLFormat.Type, InTargetableTextureFlags))
		{
			glTexImage2D(InTarget, 0, GLFormat.InternalFormat[bSRGB], InSizeX, InSizeY, 0, GLFormat.Format, GLFormat.Type, NULL);
		}

		FRHITexture* Texture = nullptr;
		EPixelFormat eFormat = (EPixelFormat)InFormat; // unint8 to enum

		FLinearColor Color = FLinearColor::Transparent;

		// Set a different clear color for each frame
		//uint8 Hue = static_cast<uint8>(255.f * (static_cast<float>(i) / CVars::GNumSwapchainImages));
		//Color = FLinearColor::FGetHSV(Hue, 0, 255);

		if (bArrayTexture)
		{
			FOpenGLTexture2DArray* Texture2DArray = new FOpenGLTexture2DArray(
				InGLRHI,
				TextureID,
				InTarget,
				GL_COLOR_ATTACHMENT0,
				InSizeX,
				InSizeY,
				InArraySize,
				1, // mips
				1, // samples
				1, // samples in tiled GPUs
				InArraySize,
				eFormat,
				false, // cubemaps?
				true,  // in allocated storage?
#if ENGINE_MINOR_VERSION == 25	
				InTargetableTextureFlags,
#else
				(ETextureCreateFlags)InTargetableTextureFlags,
#endif
			
#if ENGINE_MINOR_VERSION == 25	
				nullptr,
#endif
				FClearValueBinding(Color)
			);

			Texture = Texture2DArray;
		}
		else
		{
			FOpenGLTexture2D* Texture2D = new FOpenGLTexture2D(
				InGLRHI,
				TextureID,
				InTarget,
				GL_COLOR_ATTACHMENT0,
				InSizeX,
				InSizeY,
				0,
				1,
				1,
				1,
				1,
				eFormat,
				false,
				true,

#if ENGINE_MINOR_VERSION == 25	
				InTargetableTextureFlags,
#else
				(ETextureCreateFlags)InTargetableTextureFlags,
#endif

#if ENGINE_MINOR_VERSION == 25	
				nullptr,
#endif
				FClearValueBinding(Color)
			);

			Texture = Texture2D;
		}

		SwapChainImages[i] = Texture;

#if ENGINE_MINOR_VERSION > 25	
		OpenGLTextureAllocated(Texture, (ETextureCreateFlags)InTargetableTextureFlags);
#else
		OpenGLTextureAllocated(Texture, InTargetableTextureFlags);
#endif


		// No need to restore texture stage; leave it like this,
		// and the next draw will take care of cleaning it up; or
		// next operation that needs the stage will switch something else in on it.
	}

	if (CVars::TextureFoveationEnabled != 0 && CVars::TemporalFoveationEnabled != 0)
	{
		FOpenGLContextState& ContextState = InGLRHI->GetContextStateForCurrentContext();

		for (int CurrentImageIndex = 0; CurrentImageIndex < CVars::GNumSwapchainImages; ++CurrentImageIndex)
		{
			int PreviousImageIndex = (CurrentImageIndex + (CVars::GNumSwapchainImages - 1)) % CVars::GNumSwapchainImages;

			GLuint CurrentImageTextureID = 0, PreviousImageTextureID = 0;
			if (InTarget == GL_TEXTURE_2D_ARRAY)
			{
				CurrentImageTextureID  = *(GLuint*)SwapChainImages[CurrentImageIndex]->GetTexture2DArray()->GetNativeResource();
				PreviousImageTextureID = *(GLuint*)SwapChainImages[PreviousImageIndex]->GetTexture2DArray()->GetNativeResource();
			}
			else
			{
				CurrentImageTextureID  = *(GLuint*)SwapChainImages[CurrentImageIndex]->GetTexture2D()->GetNativeResource();
				PreviousImageTextureID = *(GLuint*)SwapChainImages[PreviousImageIndex]->GetTexture2D()->GetNativeResource();
			}

			InGLRHI->CachedSetupTextureStage(ContextState, FOpenGL::GetMaxCombinedTextureImageUnits() - 1, InTarget, CurrentImageTextureID, 0, 1);

			glTexParameteri(InTarget, GL_TEXTURE_PREVIOUS_SOURCE_TEXTURE_QCOM, PreviousImageTextureID);

			dbgprint("GL_QCOM_texture_foveated: glTexParameteri(%s (ID = %u), GL_TEXTURE_PREVIOUS_SOURCE_TEXTURE_QCOM, %u",
					 InTarget == GL_TEXTURE_2D_ARRAY ? TEXT("GL_TEXTURE_2D_ARRAY") : TEXT("GL_TEXTURE_2D"),
					 CurrentImageTextureID, PreviousImageTextureID);
		}
	}


	SwapChainImageIndex = 0;
	SetNativeResource(SwapChainImages[SwapChainImageIndex]);
#endif
}

void FQIYIVRTextureSet_OpenGL::SetNativeResource(const FTextureRHIRef& RenderTargetTexture)
{
	Resource = *(GLuint*)RenderTargetTexture->GetNativeResource();
}

FQIYIVRTextureSet_OpenGL* CreateTextureSetProxy_OpenGLES(
	class FOpenGLDynamicRHI* InGLRHI,
	GLenum InTarget,
	uint32 InSizeX,
	uint32 InSizeY,
	uint32 InArraySize,
	uint8 InFormat,
	uint32 InFlags,
	uint32 InTargetableTextureFlags,
	FTexture2DRHIRef& OutTargetableTexture,
	FTexture2DRHIRef& OutShaderResourceTexture)
{
	FQIYIVRTextureSet_OpenGL* TextureSetProxy = new FQIYIVRTextureSet_OpenGL(
		InGLRHI,
		InTarget,
		InSizeX,
		InSizeY,
		InArraySize,
		InFormat, 
		InFlags,
		InTargetableTextureFlags
	);

	// CTORNE: Slate expects the backbuffer to be a Texture2DRHI, so our FQIYIVRTextureSet_OpenGL is a FOpenGLTexture2D.
	//		   But in the case of multiview direct, our swapchain will actually consist of 2D array textures. It all seems to
	//		   work regardless, we just overwrite the texture set's Resource with a different texture ID and set the Target to
	//		   GL_TEXTURE_2D_ARRAY and that seems to be enough. Eventually OutTargetableTexture and OutShaderResourceTexture
	//		   should be made TextureRHIRefs, and the rest of the code should handle more than just 2D textures, so we can stop
	//		   using this ugly hack.
	OutTargetableTexture = OutShaderResourceTexture = TextureSetProxy->GetTexture2D();

	return TextureSetProxy;
}

GLuint TextureSetProxy_OpenGLES_GetNativeResource(FQIYIVRTextureSet_OpenGL* TextureSetProxy)
{
	return *(GLuint*)TextureSetProxy->GetRenderTarget()->GetNativeResource();
}

void TextureSetProxy_SwitchToNextElement(FQIYIVRTextureSet_OpenGL* TextureSetProxy)
{
	TextureSetProxy->SwitchToNextElement();
}

