//=============================================================================
//
//                 Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                              All Rights Reserved.
//
//=============================================================================
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "IQIYIVRHMDModule.h"
#include "QIYIVRSDKCore.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "QIYIInputFunctionLibrary.generated.h"





USTRUCT(BlueprintType)
struct FQiyiControllerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		int32 isConnect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		int32 triggerForce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		int32 gripForce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		int32 isShow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FVector2D JoyStickPos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FQuat Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		int32 BatteryLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FVector velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FVector acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FVector angVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiControllerData")
		FVector angAcceleration;

	FQiyiControllerData()
		:isConnect(0),
		triggerForce(0),
		gripForce(0),
		isShow(0),
		JoyStickPos(FVector2D::ZeroVector),
		Position(FVector::ZeroVector),
		Rotation(0.0, 0.0, 0.0, 0.0),
		BatteryLevel(0),
		velocity(FVector::ZeroVector),
		acceleration(FVector::ZeroVector),
		angVelocity(FVector::ZeroVector),
		angAcceleration(FVector::ZeroVector)
	{
	}

};

UCLASS()
class UQIYIInput :public UGameInstanceSubsystem
{
 	GENERATED_BODY()

public:
	UQIYIInput()
	{	
	}

public:

	UFUNCTION(BlueprintCallable, Category = "IQIYIInput")
		FQiyiControllerData& GetLeftControllerData()
	{
		return LeftController;
	}
	UFUNCTION(BlueprintCallable, Category = "IQIYIInput")
		FQiyiControllerData& GetRightControllerData()
	{
		return RightController;
	}

	UFUNCTION(BlueprintCallable, Category = "IQIYIInput")
	void SetControllerData()
	{
#if QIYIVR_SUPPORTED_PLATFORMS

		QiyiVRCore::QVR_GetControllerDate(&LeftData, &RightData);

		//Left
		LeftController.isConnect = (int32)LeftData.isConnect;
		LeftController.triggerForce = (int32)LeftData.triggerForce;
		LeftController.gripForce = (int32)LeftData.gripForce;
		LeftController.isShow = (int32)LeftData.isShow;
		LeftController.BatteryLevel = (int32)LeftData.batteryLevel;

		LeftController.JoyStickPos.Set(LeftData.joyStickPos.x, LeftData.joyStickPos.y);

		LeftController.Position.Set(LeftData.position.x, LeftData.position.y, LeftData.position.z);

		LeftController.Rotation = FQuat(LeftData.rotation.x, LeftData.rotation.y, LeftData.rotation.z, LeftData.rotation.w);

		LeftController.velocity.Set(LeftData.velocity.x, LeftData.velocity.y, LeftData.velocity.z);

		LeftController.acceleration.Set(LeftData.acceleration.x, LeftData.acceleration.y, LeftData.acceleration.z);

		LeftController.angVelocity.Set(LeftData.angVelocity.x, LeftData.angVelocity.y, LeftData.angVelocity.z);
 
		LeftController.angAcceleration.Set(LeftData.angAcceleration.x, LeftData.angAcceleration.y, LeftData.angAcceleration.z);



		//Right
		RightController.isConnect = (int32)RightData.isConnect;
		RightController.triggerForce = (int32)RightData.triggerForce;
		RightController.gripForce = (int32)RightData.gripForce;
		RightController.isShow = (int32)RightData.isShow;
		RightController.BatteryLevel = (int32)RightData.batteryLevel;

		RightController.JoyStickPos.Set(RightData.joyStickPos.x, RightData.joyStickPos.y);

		RightController.Position.Set(RightData.position.x, RightData.position.y, RightData.position.z);

		RightController.Rotation = FQuat(RightData.rotation.x, RightData.rotation.y, RightData.rotation.z, RightData.rotation.w);

		RightController.velocity.Set(RightData.velocity.x, RightData.velocity.y, RightData.velocity.z);

		RightController.acceleration.Set(RightData.acceleration.x, RightData.acceleration.y, RightData.acceleration.z);

		RightController.angVelocity.Set(RightData.angVelocity.x, RightData.angVelocity.y, RightData.angVelocity.z);

		RightController.angAcceleration.Set(RightData.angAcceleration.x, RightData.angAcceleration.y, RightData.angAcceleration.z);


#endif
	}

	UFUNCTION(BlueprintCallable, Category = "IQIYIInput")
		void QIYIControllerStartVibration(int32 deviceId, int32 amplitude, int32 duration)
	{
#if QIYIVR_SUPPORTED_PLATFORMS
		QiyiVRCore::QVR_ControllerStartVibration(deviceId, amplitude, duration);
#endif

	}

private:

	FQiyiControllerData LeftController;
	FQiyiControllerData RightController;

#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::ControllerData LeftData;
	QiyiVRCore::ControllerData RightData;
#endif
};