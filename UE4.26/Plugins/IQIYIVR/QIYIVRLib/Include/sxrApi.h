//=============================================================================
//! \file sxrApi.h
//
//                  Copyright (c) 2018 QUALCOMM Technologies Inc.
//                              All Rights Reserved.
//
//==============================================================================

#ifndef _SXR_API_H_
#define _SXR_API_H_

#ifndef _WIN32 // not windows 
    #define TARGET_ANDROID 
#endif

#include <stdlib.h>
#ifdef TARGET_ANDROID 
	#include <jni.h>
#endif

#define SXR_MAJOR_VERSION       4
#define SXR_MINOR_VERSION       0
#define SXR_REVISION_VERSION    3

#define SXR_MAX_RENDER_LAYERS   16

enum SxrResult
{
    // Return code for success
    SXR_ERROR_NONE = 0,

    // Return code for cases that don't fall into defined error codes
    SXR_ERROR_UNKNOWN = 1,

    // Return code to indicate that SXR isn't supported, or the necessary functionality in the system drivers
    // or VR service isn't supported.
    SXR_ERROR_UNSUPPORTED,

    // Return code for calls to functions made without first calling sxrInitialize
    SXR_ERROR_VRMODE_NOT_INITIALIZED,

    // Return code for calls made into functions that require VR mode to be in a started state
    SXR_ERROR_VRMODE_NOT_STARTED,

    // Return code for calls made into functions that require VR mode to be in a stopped state
    SXR_ERROR_VRMODE_NOT_STOPPED,

    // Return code for calls made into functions where the service fails or isn't accessible
    SXR_ERROR_QVR_SERVICE_UNAVAILABLE,

    // Error for any failures in JNI/Java calls made through the API
    SXR_ERROR_JAVA_ERROR,
};

//! \brief Events
enum sxrEventType
{
    kEventNone = 0,
    kEventSdkServiceStarting = 1,
    kEventSdkServiceStarted = 2,
    kEventSdkServiceStopped = 3,
    kEventControllerConnecting = 4,
    kEventControllerConnected = 5,
    kEventControllerDisconnected = 6,
    kEventThermal = 7,
    kEventVrModeStarted = 8,
    kEventVrModeStopping = 9,
    kEventVrModeStopped = 10,
    kEventSensorError = 11,
    kEventMagnometerUncalibrated = 12,
    kEventBoundarySystemCollision = 13,
    kEvent6dofRelocation = 14,
    kEvent6dofWarningFeatureCount = 15,
    kEvent6dofWarningLowLight = 16,
    kEvent6dofWarningBrightLight = 17,
    kEvent6dofWarningCameraCalibration = 18
};

struct ANativeWindow;

//! \brief Simple structure to hold 2-component vector data
struct sxrVector2
{
    float x,y;
};

//! \brief Simple structure to hold 3-component vector data
struct sxrVector3
{
    float x,y,z;
};

//! \brief Simple structure to hold 4-component vector data
struct sxrVector4
{
    float x,y,z,w;
};

//! \brief Simple structure to hold quaternion data
struct sxrQuaternion
{
    float x, y, z, w;
};

//! \brief Simple structure to hold 16-component matrix data
struct sxrMatrix
{
    float M[16];
};

//! \brief Simple structure to hold 4x4 matrix data
struct sxrMatrix4
{
    float M[4][4];
};

//! \brief Enum used to indicate which eye is being used
enum sxrWhichEye
{
    kLeftEye = 0,
    kRightEye,
    kNumEyes
};

//! \brief Enum used to indicate which eye to apply a render layer
enum sxrEyeMask
{
    kEyeMaskLeft = 0x00000001,
    kEyeMaskRight = 0x00000002,
    kEyeMaskBoth = 0x00000003
};

//! \brief Enum used to indicate flags passed as part of a render layer
enum sxrLayerFlags
{
    kLayerFlagNone       = 0x00000000,
    kLayerFlagHeadLocked = 0x00000001,
    kLayerFlagOpaque     = 0x00000002,
    kLayerFlagSubsampled = 0x00000004,
};

enum sxrColorSpace
{
    kColorSpaceLinear = 0,
    kColorSpaceSRGB = 1,
    kNumColorSpaces
};

//! \brief Structure containing the position and orientation of the head
struct sxrHeadPose
{
    sxrQuaternion   rotation;
    sxrVector3      position;
};

//! \brief Enum used to indicate valid components of an pose state
enum sxrTrackingMode
{
    kTrackingRotation = (1 << 0),
    kTrackingPosition = (1 << 1),
    kTrackingEye      = (1 << 2)
};

//! \brief Structure containing the full set of pose data
struct sxrHeadPoseState
{
    sxrHeadPose         pose;                   //!< Head pose
    int32_t             poseStatus;             //!< Bit field (sxrTrackingMode) indicating pose status
    uint64_t            poseTimeStampNs;        //!< Time stamp in which the head pose was generated (nanoseconds)
    uint64_t            poseFetchTimeNs;        //!< Time stamp when this pose was retrieved
    uint64_t            expectedDisplayTimeNs;  //!< Expected time when this pose should be on screen (nanoseconds)
};

//! \brief Structure containing flags indicating data valididty of an eye pose
enum sxrEyePoseStatus
{
    kGazePointValid             = (1 << 0),
    kGazeVectorValid            = (1 << 1),
    kEyeOpennessValid           = (1 << 2),
    kEyePupilDilationValid      = (1 << 3),
    kEyePositionGuideValid      = (1 << 4),
    kEyeBlinkValid              = (1 << 5)
};

//! \brief State of the foveated gaze
enum svrFoveatedEyeGazeState
{
    //kFoveatedEyeGazeStateInvalid,               //!< Foveated gaze is not supported
    kFoveatedEyeGazeStateTracking,              //!< The user is being tracked and the value has been updated.
    kFoveatedEyeGazeStateExtrapolated,          //!< The user is not being tracked and the value has been extrapolated from historical data.
    kFoveatedEyeGazeStateLastKnown,             //!< The user is not being tracked and the value is a repeat of the last tracked value.
    kFoveatedEyeGazeStateMax,
};

//! \brief Structure containing flags modifying data type of requested eye pose
enum sxrEyePoseFlags
{
    kEyeTrackingDataSynced = (1 << 0),
};

//! \brief Structure containing the eye tracking data
struct sxrEyePoseState
{
    int64_t             timestamp;                  //!< Timestamp for the eye pose

    int32_t             leftEyePoseStatus;          //!< Bit field (sxrEyePoseStatus) indicating left eye pose status
    int32_t             rightEyePoseStatus;         //!< Bit field (sxrEyePoseStatus) indicating right eye pose status
    int32_t             combinedEyePoseStatus;      //!< Bit field (sxrEyePoseStatus) indicating combined eye pose status
    
    float               leftEyeGazePoint[3];        //!< Left Eye Gaze Point
    float               rightEyeGazePoint[3];       //!< Right Eye Gaze Point
    float               combinedEyeGazePoint[3];    //!< Combined Eye Gaze Point (HMD center-eye point)

    float               leftEyeGazeVector[3];       //!< Left Eye Gaze Point
    float               rightEyeGazeVector[3];      //!< Right Eye Gaze Point
    float               combinedEyeGazeVector[3];   //!< Comnbined Eye Gaze Vector (HMD center-eye point)

    bool                leftEyeBlink;               //!< Left eye blink status (true = eye is closed, false = eye is open)
    bool                rightEyeBlink;              //!< Right eye blink status (true = eye is closed, false = eye is open)

    float               leftEyeOpenness;            //!< Left eye value between 0.0 and 1.0 where 1.0 means fully open and 0.0 closed.
    float               rightEyeOpenness;           //!< Right eye value between 0.0 and 1.0 where 1.0 means fully open and 0.0 closed.

    float               leftEyePupilDilation;       //!< Left eye value in millimeters indicating the pupil dilation
    float               rightEyePupilDilation;      //!< Right eye value in millimeters indicating the pupil dilation

    float               leftEyePositionGuide[3];    //!< Position of the inner corner of the left eye in meters from the HMD center-eye coordinate system's origin.
    float               rightEyePositionGuide[3];   //!< Position of the inner corner of the right eye in meters from the HMD center-eye coordinate system's origin.

    int                 foveatedEyeGazeState;       //!< svrFoveatedEyeGazeState
    float               foveatedEyeGazeVector[3];   //!< Foveated Eye Gaze Vector (HMD center-eye point)
};

//! \brief Enum used for indicating the CPU/GPU performance levels
//! \sa sxrBeginVr, sxrSetPerformanceLevels
enum sxrPerfLevel
{
    kPerfSystem     = 0,            //!< System defined performance level (default)
    kPerfMinimum    = 1,            //!< Minimum performance level 
    kPerfMedium     = 2,            //!< Medium performance level 
    kPerfMaximum    = 3,            //!< Maximum performance level
    kNumPerfLevels
};

//! \brief Flags used to set SXR options
enum sxrOptionFlags
{
    kProtectedContent = (1 << 0),
    kMotionAwareFrames = (1 << 1),
    kFoveationSubsampled = (1 << 2),
    kEnableCameraLayer = (1 << 3),
    kEnable3drOcclusion = (1 << 4),
};

//! \brief Structure containing parameters needed to enable VR mode
//! \sa sxrBeginVr, sxrOptionFlags
struct sxrBeginParams
{
    int32_t         mainThreadId;           //!< Thread Id of the primary render thread
    sxrPerfLevel    cpuPerfLevel;           //!< Desired CPU performance level
    sxrPerfLevel    gpuPerfLevel;           //!< Desired GPU performance level
    ANativeWindow*  nativeWindow;           //!< Pointer to the Android native window
    int32_t         optionFlags;            //!< Flags to specify SXR options (sxrOptionFlags)
    sxrColorSpace   colorSpace;             //!< Color space to utilize for the display surface
};

//! \brief Options which can be set when submitting a frame to modify the behavior of asynchronous time warp
//! \sa sxrSubmitFrame
enum sxrFrameOption
{
    kDisableDistortionCorrection    = ( 1 << 0 ),   //!< Disables the lens distortion correction (useful for debugging)
    kDisableReprojection            = ( 1 << 1 ),   //!< Disables re-projection
    kEnableMotionToPhoton           = ( 1 << 2 ),   //!< Enables motion to photon testing 
    kDisableChromaticCorrection     = ( 1 << 3 )    //!< Disables the lens chromatic aberration correction (performance optimization)
};

//! \brief Enum used to indicate the type of texture passed in as a render layer
//! \sa sxrSubmitFrame
enum sxrTextureType
{
    kTypeTexture = 0,               //!< Standard texture
    kTypeTextureArray,              //!< Standard texture array (Left eye is first layer, right eye is second layer)
    kTypeImage,                     //!< EGL Image texture
    kTypeEquiRectTexture,           //!< Equirectangular texture
    kTypeEquiRectImage,             //!< Equirectangular Image texture
    kTypeCubemapTexture,            //!< Cubemap texture (Not supporting cubemap image)
    kTypeVulkan,                    //!< Vulkan texture
    kTypeCamera                     //!< Camera texture
};

//! \brief Information about texture if type is kTypeVulkan
//! \sa sxrSubmitFrame
struct sxrVulkanTexInfo
{
    uint32_t            memSize;
    uint32_t            width;
    uint32_t            height;
    uint32_t            numMips;
    uint32_t            bytesPerPixel;
    uint32_t            renderSemaphore;
};


//! \brief Enum used to indicate the type of warp/composition that should be used for a frame
enum sxrWarpType
{
    kSimple                         //!< Basic single layer (world) warp 
};

//! \brief Enumeration of possible warp mesh types
//! \sa sxrDeviceInfo
enum sxrWarpMeshType
{
    kMeshTypeColumsLtoR = 0,    // Columns Left to Right
    kMeshTypeColumsRtoL,        // Columns Right to Left
    kMeshTypeRowsTtoB,          // Rows Top to Bottom
    kMeshTypeRowsBtoT,          // Rows Bottom to Top
};

//! \brief Enumeration of possible warp meshes
//! \sa sxrSetWarpMesh, sxrWarpMeshType
enum sxrWarpMeshEnum
{
    kMeshEnumLeft = 0,  // Column mesh for left half of screen
    kMeshEnumRight,     // Column mesh for right half of screen
    kMeshEnumUL,        // Row mesh for Upper Left part of the screen
    kMeshEnumUR,        // Row mesh for Upper Right part of the screen
    kMeshEnumLL,        // Row mesh for Lower Left part of the screen
    kMeshEnumLR,        // Row mesh for Lower Right part of the screen
    kWarpMeshCount
};

//! \brief Render layer screen position and UV coordinates
//! \sa sxrSubmitFrame
struct sxrLayoutCoords
{
    float               LowerLeftPos[4];                        //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = W-Component
    float               LowerRightPos[4];                       //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = W-Component
    float               UpperLeftPos[4];                        //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = W-Component
    float               UpperRightPos[4];                       //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = W-Component

    float               LowerUVs[4];                            //!< [0,1] = Lower Left UV values; [2,3] = Lower Right UV values
    float               UpperUVs[4];                            //!< [0,1] = Upper Left UV values; [2,3] = Upper Right UV values

    float               TransformMatrix[16];                    //!< Column major uv transform matrix data. Applies to video textures (see SurfaceTexture::getTransformMatrix())
};

//! \brief Description of render layers
//! \sa sxrSubmitFrame
struct sxrRenderLayer
{
    int32_t             imageHandle;        //!< Handle to the texture/image to be rendered
    sxrTextureType      imageType;          //!< Type of texture: Standard Texture or EGL Image
    sxrLayoutCoords     imageCoords;        //!< Layout of this layer on the screen
    sxrEyeMask          eyeMask;            //!< Determines which eye[s] receive this render layer
    uint32_t            layerFlags;         //!< Flags applied to this render layer
    sxrVulkanTexInfo    vulkanInfo;         //!< Information about the data if it is a Vulkan texture
};


//! \brief Per-frame data needed for time warp, distortion/aberration correction
//! \sa sxrSubmitFrame
struct sxrFrameParams
{
    int32_t             frameIndex;                             //!< Frame Index
    int32_t             minVsyncs;                              //!< Minimum number of vysnc events before displaying the frame (1=display refresh, 2=half refresh, etc...)

    sxrRenderLayer      renderLayers[SXR_MAX_RENDER_LAYERS];    //!< Description of each render layer

    uint32_t            frameOptions;                           //!< Options for adjusting the frame warp behavior (bitfield of sxrFrameOption)
    sxrHeadPoseState    headPoseState;                          //!< Head pose state used to generate the frame  
    sxrWarpType         warpType;                               //!< Type of warp to be used on the frame
    float               fieldOfView;                            //!< Field of view used to generate this frame (larger than device fov to provide timewarp margin)
};

//! \brief Initialization parameters that are constant over the life-cycle of the application
//! \sa sxrInitialize
struct sxrInitParams
{
#ifdef TARGET_ANDROID 
    JavaVM*         javaVm;                 //!< Java Virtual Machine pointer
    JNIEnv*         javaEnv;                //!< Java Environment
    jobject         javaActivityObject;     //!< Reference to the Android activity
#endif
};

//! \brief View Frustum.  These values are based on physical device properties, except the far plane is arbitrary
//! \sa sxrSubmitFrame
struct sxrViewFrustum
{
    float               left;           //!< Left Plane of Frustum
    float               right;          //!< Right Plane of Frustum
    float               top;            //!< Top Plane of Frustum
    float               bottom;         //!< Bottom Plane of Frustum

    float               near;           //!< Near Plane of Frustum
    float               far;            //!< Far Plane of Frustum (Arbitrary)

    sxrVector3          position;       //!< Position Offset of Frustum
    sxrQuaternion       rotation;       //!< Rotation Quaternion of Frustum
};

struct sxrFoveation
{
    sxrVector2 gain;             //!< Foveation Gain Rate [1, ...]
    float area;                  //!< Foveation Area Size [0, ...]
    float minimum;               //!< Foveation Minimum Resolution [1, 1/2, 1/4, ..., 1/16, 0]
};

//! \brief Basic device information to allow the client code to optimally setup their simulation and rendering pipelines
struct sxrDeviceInfo
{
    int32_t         displayWidthPixels;         //!< Physical width of the display (pixels)
    int32_t         displayHeightPixels;        //!< Physical height of the display (pixels)
    float           displayRefreshRateHz;       //!< Refresh rate of the display
    int32_t         displayOrientation;         //!< Display orientation (degrees at initialization - 0,90,180,270)
    int32_t         targetEyeWidthPixels;       //!< Recommended eye buffer width (pixels)
    int32_t         targetEyeHeightPixels;      //!< Recommended eye buffer height (pixels)
    float           targetFovXRad;              //!< Recommended horizontal FOV
    float           targetFovYRad;              //!< Recommended vertical FOV
    sxrViewFrustum  leftEyeFrustum;             //!< Recommended Frustum information for left eye
    sxrViewFrustum  rightEyeFrustum;            //!< Recommended Frustum information for right eye
    float           targetEyeConvergence;       //!< Recommended eye convergence
    float           targetEyePitch;             //!< Recommended eye pitch
    int32_t         deviceOSVersion;            //!< Android OS Version of the device
    sxrWarpMeshType warpMeshType;               //!< Type of mesh used to render eye buffer
    sxrFoveation    lowFoveation;               //!< Low foveation values
    sxrFoveation    medFoveation;               //!< Medium foveation values
    sxrFoveation    highFoveation;              //!< High foveation values

    // Tracking Camera
    float           trackingCalibration[12];    //!< Tracking Camera calibration 4x3 matrix
    float           trackingPrincipalPoint[2];  //!< Tracking Camera principal point
    float           trackingFocalLength[2];     //!< Tracking Camera focal length
    float           trackingDistortion[8];      //!< Tracking Camera radial distortion

    // Tracking capabilities
    uint64_t        trackingCapabilities;       //!< Qvr service capabilities: Eye Tracking
};

enum sxrThermalLevel
{
    kSafe,
    kLevel1,
    kLevel2,
    kLevel3,
    kCritical,
    kNumThermalLevels
};

enum sxrThermalZone
{
    kCpu,
    kGpu,
    kSkin,
    kNumThermalZones
};

struct sxrEventData_Thermal
{
    sxrThermalZone  zone;               //!< Thermal zone
    sxrThermalLevel level;              //!< Indication of the current zone thermal level
};

typedef union 
{
    sxrEventData_Thermal    thermal;
    uint32_t                data[2];
} sxrEventData;

struct sxrEvent
{
    sxrEventType    eventType;              //!< Type of event
    uint32_t        deviceId;               //!< An identifier for the device that generated the event (0 == HMD)
    float           eventTimeStamp;         //!< Time stamp for the event in seconds since the last sxrBeginVr call
    sxrEventData    eventData;              //!< Event specific data payload
};

//! \brief Events to use in sxrControllerSendMessage
enum sxrControllerMessageType
{
    kControllerMessageRecenter = 0,
    kControllerMessageVibration = 1
};

//! \brief Query Values
enum sxrControllerQueryType
{
    kControllerQueryBatteryRemaining = 0,
    kControllerQueryControllerCaps = 1
};

//! Controller Connection state
enum sxrControllerConnectionState {
    kNotInitialized = 0,
    kDisconnected = 1,
    kConnected = 2,
    kConnecting = 3,
    kError = 4
};

//! Controller Touch button enumerations
namespace sxrControllerTouch {
  enum {
    None                = 0x00000000,
    One                 = 0x00000001,
    Two                 = 0x00000002,
    Three               = 0x00000004,
    Four                = 0x00000008,
    PrimaryThumbstick   = 0x00000010,
    SecondaryThumstick  = 0x00000020,
    Any                 = ~None
  };
}

//! Controller Trigger enumerations
namespace sxrControllerAxis1D {
enum {
  PrimaryIndexTrigger   = 0x00000000,
  SecondaryIndexTrigger = 0x00000001,
  PrimaryHandTrigger    = 0x00000002,
  SecondaryHandTrigger  = 0x00000003
};
}

//! Controller Joystick enumerations
namespace sxrControllerAxis2D {
enum {
  PrimaryThumbstick     = 0x00000000,
  SecondaryThumbstick   = 0x00000001
};
}

//! Controller Button enumerations
namespace sxrControllerButton {
enum {
  None                    = 0x00000000,
  One                     = 0x00000001,
  Two                     = 0x00000002,
  Three                   = 0x00000004,
  Four                    = 0x00000008,
  DpadUp                  = 0x00000010,
  DpadDown                = 0x00000020,
  DpadLeft                = 0x00000040,
  DpadRight               = 0x00000080,
  Start                   = 0x00000100,
  Back                    = 0x00000200,
  PrimaryShoulder         = 0x00001000,
  PrimaryIndexTrigger     = 0x00002000,
  PrimaryHandTrigger      = 0x00004000,
  PrimaryThumbstick       = 0x00008000,
  PrimaryThumbstickUp     = 0x00010000,
  PrimaryThumbstickDown   = 0x00020000,
  PrimaryThumbstickLeft   = 0x00040000,
  PrimaryThumbstickRight  = 0x00080000,
  SecondaryShoulder       = 0x00100000,
  SecondaryIndexTrigger   = 0x00200000,
  SecondaryHandTrigger    = 0x00400000,
  SecondaryThumbstick     = 0x00800000,
  SecondaryThumbstickUp   = 0x01000000,
  SecondaryThumbstickDown = 0x02000000,
  SecondaryThumbstickLeft = 0x04000000,
  SecondaryThumbstickRight = 0x08000000,
  Up                      = 0x10000000,
  Down                    = 0x20000000,
  Left                    = 0x40000000,
  Right                   = 0x80000000,
  Any                     = ~None
};
}

// Current state of the controller
struct sxrControllerState {
    //! Orientation
    sxrQuaternion   rotation;

    //! Position
    sxrVector3      position;
    
    //! Gyro
    sxrVector3      gyroscope;
    
    //! Accelerometer
    sxrVector3      accelerometer;
    
    //! timestamp
    uint64_t        timestamp;

    //! All digital button states as bitflags
    uint32_t        buttonState;   //!< all digital as bit flags
    
    //! Touchpads, Joysticks
    sxrVector2      analog2D[4];       //!< analog 2D's

    //! Triggers
    float           analog1D[8];       //!< analog 1D's

    //! Whether the touchpad area is being touched.
    uint32_t        isTouching;
     
    //! Controller Connection state
    sxrControllerConnectionState connectionState;
};

// Caps of the Controller.
struct sxrControllerCaps {
    //! Device Manufacturer
    char deviceManufacturer[64];
    
    //! Device Identifier from the module
    char deviceIdentifier[64];
    
    //! Controller Capabilities
    uint32_t caps; //0 bit = Provides Rotation; 1 bit = Position;

    //! Enabled Buttons Bitfield
    uint32_t activeButtons;
    
    //! Active 2D Analogs Bitfield
    uint32_t active2DAnalogs;
    
    //! Active 1D Analogs Bitfield
    uint32_t active1DAnalogs;
    
    //! Active Touch Buttons
    uint32_t activeTouchButtons;
};

#ifndef SXRP_EXPORT
    #define SXRP_EXPORT
#endif

#ifdef __cplusplus 
extern "C" {
#endif

//! \brief Returns the VR SDK version string
SXRP_EXPORT const char* sxrGetVersion();

//! \brief Returns the VR service version string
SXRP_EXPORT SxrResult sxrGetXrServiceVersion(char *pRetBuffer, int bufferSize);

//! \brief Returns the VR client version string
SXRP_EXPORT SxrResult sxrGetXrClientVersion(char *pRetBuffer, int bufferSize);

//! \brief Initializes VR components 
//! \param pInitParams sxrInitParams structure
SXRP_EXPORT SxrResult sxrInitialize(const sxrInitParams* pInitParams);

//! \brief Releases VR components
SXRP_EXPORT SxrResult sxrShutdown();

//! \brief Queries for device specific information
//! \return sxrDeviceInfo structure containing device specific information (resolution, fov, etc..)
SXRP_EXPORT sxrDeviceInfo   sxrGetDeviceInfo();

//! \brief Requests specific brackets of CPU/GPU performance
//! \param cpuPerfLevel Requested performance level for CPU
//! \param gpuPerfLevel Requested performance level for GPU
SXRP_EXPORT SxrResult sxrSetPerformanceLevels(sxrPerfLevel cpuPerfLevel, sxrPerfLevel gpuPerfLevel);

//! \brief Set the occlusion mesh
//! \param numMeshVertices Number of mesh vertices
//! \param numMeshIndices Number of mesh indicies
//! \param meshVertices Mesh vertices of x, y, z floats
//! \param meshIndices Mesh indicies of unsigned int
SXRP_EXPORT SxrResult sxrSetOcclusionMesh(int numMeshVertices, int numMeshIndices, float* meshVertices, unsigned int* meshIndices);

//! \brief Enables XR services
//! \param pBeginParams sxrBeginParams structure
SXRP_EXPORT SxrResult sxrBeginXr(const sxrBeginParams* pBeginParams);

//! \brief Disables XR services
SXRP_EXPORT SxrResult sxrEndXr();

//! \brief Calculates a predicted time when the current frame would be displayed. Assumes a pipeline depth of 1
//! \return Predicted display time for the current frame in milliseconds
SXRP_EXPORT float sxrGetPredictedDisplayTime();

//! \brief Calculates a predicted time when the current frame would be displayed.  Allows for specifying the current depth of the pipeline
//! \return Predicted display time for the current frame in milliseconds
SXRP_EXPORT float sxrGetPredictedDisplayTimePipelined(unsigned int depth);

//! \brief Calculates a predicted head pose
//! \param predictedTimeMs Time ahead of the current time in ms to predict a head pose for
//! \return The predicted head pose and relevant pose state information
SXRP_EXPORT sxrHeadPoseState sxrGetPredictedHeadPose( float predictedTimeMs );

//! \brief Retrieves a historic head pose
//! \param timestampNs Time in ns to retrieve a head pose for
//! \return The head pose and relevant pose state information
SXRP_EXPORT sxrHeadPoseState sxrGetHistoricHeadPose(int64_t timestampNs);

//! \brief Retrieves a current eye pose
//! \param flags to indicate the type (e.g. synced or not)
//! \return The eye pose
SXRP_EXPORT SxrResult sxrGetEyePose(sxrEyePoseState *pReturnPose, int32_t flags=0);

//! \brief Retrieve the service capabilities for eye tracking
SXRP_EXPORT SxrResult sxrGetTrackingCapabilities(uint64_t *pEyeCapabilities);

//! \brief Retrieve the qvr service data transform into sxr device coordinates
SXRP_EXPORT SxrResult sxrGetQvrDataTransform(sxrMatrix *pQvrTransform);

//! \brief Retrieve the recenter transform in sxr device coordinates
SXRP_EXPORT SxrResult sxrGetRecenterPose(sxrVector3 *pRecenterPosition, sxrQuaternion *pRecenterRotation);

//! \brief Recenters the head position and orientation at the current values
SXRP_EXPORT SxrResult sxrRecenterPose();

//! \brief Set the recenter transform in sxr device coordinates last recenter
SXRP_EXPORT SxrResult sxrSetLastRecenterPose(sxrVector3 *pPosition, sxrQuaternion *pRotation);

//! \brief Recenters the head position at the current position
SXRP_EXPORT SxrResult sxrRecenterPosition();

//! \brief Recenters the head orientation (Yaw only) at the current value
SXRP_EXPORT SxrResult sxrRecenterOrientation(bool yawOnly=true);

//! \brief Returns the supported tracking types
//! \return Bitfield of sxrTrackingType values indicating the supported tracking modes
SXRP_EXPORT uint32_t sxrGetSupportedTrackingModes();

//! \brief Sets the current head tracking mode
//! \param trackingModes Bitfield of sxrTrackingType values indicating the tracking modes to enable
SXRP_EXPORT SxrResult sxrSetTrackingMode(uint32_t trackingModes);

//! \brief Returns the current head tracking mode
//! \return trackingMode Bitfield of sxrTrackingType values indicating the tracking modes enabled
SXRP_EXPORT uint32_t sxrGetTrackingMode();

//! \brief Polls for an available event.  If event(s) are present pEvent will be filled with the event details
//! \return true if an event was present, false if not
SXRP_EXPORT bool sxrPollEvent(sxrEvent *pEvent);

//! \brief Called after eye buffer is bound but before game rendering starts
//! \param whichEye Which eye is being rendered
//! \param imageType Type of eye render buffer
SXRP_EXPORT SxrResult sxrBeginEye(sxrWhichEye whichEye, sxrTextureType imageType = kTypeTexture);

//! \brief Called after eye buffer is rendered but before frame is submitted
//! \param whichEye Which eye is being rendered
//! \param imageType Type of eye render buffer
SXRP_EXPORT SxrResult sxrEndEye(sxrWhichEye whichEye, sxrTextureType imageType = kTypeTexture);

//! \brief Called after eye buffer is bound but before game rendering starts
//! \param whichEye Which eye is being rendered
//! \param pEyeProj is eye projection matrix
//! \param pEyeView is eye view matrix
//! \param pModelMat is model transform matrix (optional)
//! \param frontFaceMode is the vertex winding order 0/CCW (default), ~0/CW
//! \param imageType Type of eye render buffer (optional)
SXRP_EXPORT SxrResult sxrOccludeEye(sxrWhichEye whichEye, float *pEyeProj, float *pEyeView, float *pModelMat = NULL, int frontFaceMode = 0, sxrTextureType imageType = kTypeTexture);

//! \brief Submits a frame to asynchronous time warp
//! \param pFrameParams sxrFrameParams structure
SXRP_EXPORT SxrResult sxrSubmitFrame(const sxrFrameParams* pFrameParams);

//! \brief Starts Tracking 
//! \param controllerDesc Controller Description
//! \return handle for the controller
SXRP_EXPORT int sxrControllerStartTracking(const char* controllerDesc);

//! \brief Stops tracking the controller
//! \param controllerHandle handle for the controller
SXRP_EXPORT void sxrControllerStopTracking(int controllerHandle);

//! \brief Get the current state of the controller
//! \param controllerHandle handle for the controller
SXRP_EXPORT sxrControllerState sxrControllerGetState(int controllerHandle, int controllerSpace = 0);

//! \brief Send a message to the controller
//! \param controllerHandle handle for the controller
//! \param what type of event
//! \param arg1 argument for the event
//! \param arg2 argument for the event
SXRP_EXPORT void sxrControllerSendMessage(int controllerHandle, int what, int arg1, int arg2);

//! \brief make a query to the controller
//! \param controllerHandle handle for the controller
//! \param what info
//! \param memory memory to fill in
//! \param memorySize size of the memory to fill in
//! \return number of bytes written into the memory
SXRP_EXPORT int sxrControllerQuery(int controllerHandle, int what, void* memory, unsigned int memorySize);

//! \brief Replaces the current Time Warp mesh the supplied mesh
//! \param whichMesh Which mesh is to be replaced
//! \param pVertexData 15 float values per vertex: Position = 3; R|G|B UV values 4 each (Fourth value is Vignette multiplier [0,1]). NULL reverts the override.
//! \param vertexSize Size of pVertexData
//! \param nVertices Number of vertices contained in pVertexData
//! \param pIndices Pointer to indice data. Expected to be used with GL_TRIANGLES
//! \param nIndices Number of indices contained in pIndices. Expected to be used with GL_TRIANGLES
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNSUPPORTED if vertexSize is incorrect; SXR_ERROR_UNKNOWN for any other error
//! \sa sxrWarpMeshType, sxrWarpMeshEnum
SXRP_EXPORT SxrResult sxrSetWarpMesh(sxrWarpMeshEnum whichMesh, void *pVertexData, int vertexSize, int nVertices, unsigned int* pIndices, int nIndices);

//! \brief Check whether the 3DR system is enabled
//! \return True/False for Enabled/Disabled
SXRP_EXPORT bool sxrIs3drEnabled();

//! \brief Gets triangle data for the occlusion area mesh
//! \param whichEye which eye to get the occlusion mesh for
//! \param nTriangleCount the number of triangles needed for the mesh, set to required size if pTriangles == NULL
//! \param pTriangles buffer to store occlusion mesh data
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNKNOWN for any other error
SXRP_EXPORT SxrResult sxrGetOcclusionMesh(sxrWhichEye whichEye, int *pTriangleCount, int *pVertexStride, float *pTriangles);

//! \brief Check whether the Occlusion system is enabled
//! \return True/False for Enabled/Disabled
SXRP_EXPORT bool sxrIsOcclusionEnabled();

//! \brief Check whether the Boundary system is enabled
//! \return True/False for Enabled/Disabled
SXRP_EXPORT bool sxrIsBoundarySystemEnabled();

//! \brief Enable/Disable the Boundary system
//! \param enableBoundary Whether to enable or disable the Boundary system
SXRP_EXPORT void sxrEnableBoundarySystem(bool enableBoundary = true);

//! \brief Enable/Disable the forced display of Boundary system
//! \param enableBoundary Whether to enable or disable the forced display of Boundary system
SXRP_EXPORT void sxrForceDisplayBoundarySystem(bool forceDisplayBoundary = true);

//! \brief Set parameters for the Boundary system
//! \param pMinValues Real world minimum physical boundaries (Meters. 0 = X-Position; 1 = Y-Position; 2 = Z-Position)
//! \param pMaxValues Real world maximum physical boundaries (Meters. 0 = X-Position; 1 = Y-Position; 2 = Z-Position)
//! \param visibilityRadius Distance from physical boundaries when visual notification starts to appear
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNKNOWN for any other error
SXRP_EXPORT SxrResult sxrSetBoundaryParameters(float *pMinValues, float *pMaxValues, float visibilityRadius);

//! \brief Get parameters for the Boundary system
//! \param pMinValues Real world minimum physical boundaries (Meters. 0 = X-Position; 1 = Y-Position; 2 = Z-Position)
//! \param pMaxValues Real world maximum physical boundaries (Meters. 0 = X-Position; 1 = Y-Position; 2 = Z-Position)
//! \param pVisibilityRadius Distance from physical boundaries when visual notification starts to appear
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNKNOWN for any other error
SXRP_EXPORT SxrResult sxrGetBoundaryParameters(float *pMinValues, float *pMaxValues, float *pVisibilityRadius);

//! \brief Callback to Presentation system which signals a new presentation Surface configuration
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNKNOWN for any other error
SXRP_EXPORT SxrResult sxrSetPresentationSurfaceChanged(ANativeWindow* pWindow, int format, int width, int height);

//! \brief Callback to Presentation system which signals a destroyed presentation Surface
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNKNOWN for any other error
SXRP_EXPORT SxrResult sxrSetPresentationSurfaceDestroyed();

//! \brief Set extra latency mode on or off, this mode default is on;
//! \return SXR_ERROR_NONE for success; SXR_ERROR_UNKNOWN for any other error
SXRP_EXPORT SxrResult sxrSetExtraLatencyMode(bool mode);

//! \brief Check whether RemoteVR is enabled
//! \return True/False for Enabled/Disabled
SXRP_EXPORT bool sxrIsRemoteRenderEnabled();
//+qiyi
SXRP_EXPORT void startQiyiPresentationThread();
SXRP_EXPORT void startQiyiProjectionThread();
SXRP_EXPORT void startQiyiScreenShotThread();
SXRP_EXPORT SxrResult qiyiPorjectionSurfaceChanged(ANativeWindow* pWindow, int format, int width, int height);
SXRP_EXPORT SxrResult qiyiPorjectionSurfaceDestroyed();
SXRP_EXPORT SxrResult qiyiScreenShotSurfaceChanged(ANativeWindow* pWindow, int format, int width, int height);
SXRP_EXPORT SxrResult qiyiScreenShotSurfaceDestroyed();
SXRP_EXPORT void stopPresentationIsBegin();
//-qiyi
#ifdef __cplusplus 
}
#endif

#endif //_SXR_API_H_
