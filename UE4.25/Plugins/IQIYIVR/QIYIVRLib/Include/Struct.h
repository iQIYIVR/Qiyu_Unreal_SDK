//
// Created by xingzhishan on 2020/11/28.
//

#ifndef __STRUCT_H__
#define __STRUCT_H__

#include "Define.h"

namespace QiyiVRCore {

    ENUM ENUM_FLOOR_EYE_TYPE
    {
        Eye =0,
        Floor= 1,
    };

    ENUM ENUM_HANDEDNESS
    {
        RightHanded,
        LeftHanded,
        Default = RightHanded,
    };

    ENUM ENUM_BUTTON
    {
        None = 0,
        Trigger = 0x01,
        Grip = 0x02,
        A = 0x10,
        B = 0x20,
        Home = 0x40,
        JoyStick = 0x80,
    };

    typedef struct Vector2 {
        float x;
        float y;
    } Vector2f;

    typedef struct Vector3 {
        float x;
        float y;
        float z;
    } Vector3f;

    typedef struct Quaternion {
        float x;
        float y;
        float z;
        float w;
    } Quatf;

    ENUM ENUM_CONTROLLER_NODE {
        None = -1,
        EyeLeft = 0,
        EyeRight = 1,
        EyeCenter = 2,
        HandLeft = 3,
        HandRight = 4,
        TrackerZero = 5,
        TrackerOne = 6,
        TrackerTwo = 7,
        TrackerThree = 8,
        Head = 9,
        DeviceObjectZero = 10,
        Count,
    };

    ENUM ENUM_BOUNDARY_TYPE {
        OuterBoundary = 1,
        PlayArea = 2,
    };

    struct BoundaryTestResult {
        Bool IsTriggering;
        float ClosestDistance;
        Vector3f ClosestPoint;
        Vector3f ClosestPointNormal;
    };

    struct ControllerData {
        int isConnect;      //是否连接
        int button;         //按键状态
        int buttonTouch;    //触摸状态
        int batteryLevel;   //电池电量
        int triggerForce;   //trigger力度
        int gripForce;      //grip力度
        int isShow;        //是否显示手柄
        Vector2 joyStickPos;        //摇杆坐标
        Vector3 position;           //手柄位置
        Quaternion rotation;        //手柄四元数
        Vector3 velocity;           //线速度
        Vector3 acceleration;       //线加速度
        Vector3 angVelocity;        //角速度
        Vector3 angAcceleration;    //角加速度
    };
}

#endif 
