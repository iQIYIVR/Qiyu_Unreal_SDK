//
// Created by xingzhishan on 2020/11/28.
//

#ifndef __DEFINE_H__
#define __DEFINE_H__

namespace QiyiVRCore {

#ifdef WINDOWS
#ifdef DLL_EXPORTS
#define QIYIVRSDK_EXPORT _declspec(dllexport)

#else
#define QIYIVRSDK_EXPORT _declspec(dllimport)
#endif
#else
#define QIYIVRSDK_EXPORT
#endif

#define QIYI_ANDROID_PLATFORM !WITH_EDITOR

#define ENUM enum class
#define  FALSE 0
#define  TRUE 1

    typedef int int32;
    typedef int32 Bool;
    typedef long long int64;
    typedef unsigned long long uint64;
    typedef const char *PCChar;
    typedef void *IntPtr;
    typedef void(*MessageCallBack)(uint64 messageID, PCChar result, int32 resultSize);
    typedef void(*BoundaryCallBack)(int bType);

#define EXTERN_VAR(type, var) \
    extern "C" { extern type var; }
}
#endif
