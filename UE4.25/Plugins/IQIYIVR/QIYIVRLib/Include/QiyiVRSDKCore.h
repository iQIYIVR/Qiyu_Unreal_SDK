//
// Created by xingzhishan on 2020/11/26.
//

#ifndef QIYIVRSDKCORE_QIYIVRSDKCORE_H
#define QIYIVRSDKCORE_QIYIVRSDKCORE_H


#include "Struct.h"
#include "sxrApi.h"

#if QIYI_ANDROID_PLATFORM
#include "jni.h"
#endif

namespace QiyiVRCore {
    extern "C" {

//测试接口
    QIYIVRSDK_EXPORT const char *QVR_TestString(const char *result);
    QIYIVRSDK_EXPORT void QVR_TestMessageCallBack(uint64 requestID, const char *parma);

#if QIYI_ANDROID_PLATFORM
    QIYIVRSDK_EXPORT void QVR_Init(jobject activity,Bool isMultiThreadedRender, MessageCallBack funPtr_request, MessageCallBack funPtr_listener);
#endif

    QIYIVRSDK_EXPORT void QVR_Update(float deltaTime);
    QIYIVRSDK_EXPORT void QVR_OnApplicationPause(bool pause);
    QIYIVRSDK_EXPORT void QVR_SendHomeEvent();

//平台相关
    QIYIVRSDK_EXPORT void QVR_InitQiyuSDK(PCChar app_id, PCChar developer_id, PCChar app_secret, PCChar sign_key);
    QIYIVRSDK_EXPORT void QVR_GetAppUpdateInfo(uint64 requestID, PCChar app_id, PCChar curVersion);
    QIYIVRSDK_EXPORT Bool QVR_IsAccountLogin();
    QIYIVRSDK_EXPORT void QVR_GetQiyuAccountInfo(uint64 requestID);
    QIYIVRSDK_EXPORT void QVR_LaunchOtherApp(PCChar app_id, PCChar key, PCChar value);
    QIYIVRSDK_EXPORT void QVR_GetDeepLink(uint64 requestID);
    QIYIVRSDK_EXPORT int32 QVR_GetHand();
    QIYIVRSDK_EXPORT void QVR_SetHand(int32 type);
    QIYIVRSDK_EXPORT void QVR_LaunchHome(PCChar key, PCChar value);
//手柄
#if QIYI_ANDROID_PLATFORM
    QIYIVRSDK_EXPORT void QVR_GetControllerDate(ControllerData *left, ControllerData *right);
#endif
    QIYIVRSDK_EXPORT Bool QVR_GetControllerIsInit();
    QIYIVRSDK_EXPORT void QVR_ControllerBeginServer();
    QIYIVRSDK_EXPORT void QVR_ControllerEndServer();
    QIYIVRSDK_EXPORT void QVR_ControllerSendMsg(PCChar cmd);
    QIYIVRSDK_EXPORT void QVR_ControllerSetType(int32 type);
    QIYIVRSDK_EXPORT int32 QVR_ControllerGetType();
    QIYIVRSDK_EXPORT void QVR_ControllerStartVibration(int32 deviceId, int32 amplitude, int32 duration);

    QIYIVRSDK_EXPORT sxrHeadPoseState QVR_GetHeadPoseState();
    QIYIVRSDK_EXPORT int32 QVR_GetFloorEyeType();
    QIYIVRSDK_EXPORT void QVR_SetFloorEyeType(int32 type);
//虚拟围栏
	QIYIVRSDK_EXPORT Bool QVR_GetBoundaryConfigured();
	QIYIVRSDK_EXPORT BoundaryTestResult
	QVR_TestBoundaryPoint(Vector3f point, ENUM_BOUNDARY_TYPE boundaryType);
    QIYIVRSDK_EXPORT void
    QVR_GetOriginAxis(Vector3f &pos, Quatf &rot);
	QIYIVRSDK_EXPORT Bool
	QVR_GetBoundaryGeometry(ENUM_BOUNDARY_TYPE boundaryType, IntPtr points, int32 &pointsCount);
    QIYIVRSDK_EXPORT Vector3f QVR_GetBoundaryDimensions(ENUM_BOUNDARY_TYPE boundaryType);
	QIYIVRSDK_EXPORT Bool QVR_GetBoundaryVisible();
	QIYIVRSDK_EXPORT Bool QVR_SetBoundaryVisible(Bool value);
    QIYIVRSDK_EXPORT void QVR_RegisterBoundaryChangedCallback(BoundaryCallBack callBack);
    QIYIVRSDK_EXPORT void QVR_UnRegisterBoundaryChangedCallback(BoundaryCallBack callBack);

    QIYIVRSDK_EXPORT Bool QVR_GetFoveatEnable();
    QIYIVRSDK_EXPORT float QVR_GetFloorLevel();

    //其他
    QIYIVRSDK_EXPORT int32 QVR_GetUserIPD();
    };
}

#endif //QIYIVRSDKCORE_QIYIVRSDKCORE_H
