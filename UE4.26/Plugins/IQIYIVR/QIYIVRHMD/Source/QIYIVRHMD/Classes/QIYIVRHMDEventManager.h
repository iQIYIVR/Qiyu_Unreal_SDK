//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#pragma once

#include "Delegates/Delegate.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"
#include "QIYIVRHMDEventManager.generated.h"

UENUM(BlueprintType)
enum class EQIYIVRSdkServiceEventType : uint8
{
	EventSdkServiceStarting,
	EventSdkServiceStarted,
	EventSdkServiceStopped
};

UENUM(BlueprintType)
enum class EQIYIVRModeEventType : uint8
{
	EventVrModeStarted,
	EventVrModeStopping,
	EventVrModeStopped
};

UENUM(BlueprintType)
enum class EQIYIVRSensorEventType : uint8
{
	EventSensorError,
	EventMagnometerUncalibrated
};

UENUM(BlueprintType)
enum class EQIYIVRThermalLevel : uint8
{
	Safe,
	Level1,
	Level2,
	Level3,
	Critical
};


UENUM(BlueprintType)
enum class EQIYIVRThermalZone : uint8
{
	Cpu,
	Gpu,
	Skin
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQIYIVRSdkServiceDelegate, EQIYIVRSdkServiceEventType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQIYIVRThermalDelegate, EQIYIVRThermalZone, Zone, EQIYIVRThermalLevel, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQIYIVRModeDelegate, EQIYIVRModeEventType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQIYIVRSensorDelegate, EQIYIVRSensorEventType, Type);

UCLASS()
class UQIYIVRHMDEventManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(BlueprintAssignable)
		FQIYIVRSdkServiceDelegate OnQIYIVRSdkServiceDelegate;

	UPROPERTY(BlueprintAssignable)
		FQIYIVRThermalDelegate OnQIYIVRThermalDelegate;

	UPROPERTY(BlueprintAssignable)
		FQIYIVRModeDelegate OnQIYIVRModeDelegate;

	UPROPERTY(BlueprintAssignable)
		FQIYIVRSensorDelegate OnQIYIVRSensorDelegate;

public:
	static UQIYIVRHMDEventManager* GetInstance();
};