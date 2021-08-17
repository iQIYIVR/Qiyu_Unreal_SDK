//=============================================================================
//
//                 Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                              All Rights Reserved.
//
//=============================================================================

#pragma once
#include "IQIYIInputModule.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

#include "GenericPlatform/IInputInterface.h"
#include "XRMotionControllerBase.h"
#include "QIYIInputState.h"
#include "IInputDevice.h"

namespace QIYIInput
{
	class FQIYIInput;
}


class FQIYIInputModule : public IQIYIInputModule
{

	TWeakPtr<QIYIInput::FQIYIInput> QIYIInputDevice;

	virtual void StartupModule() override;

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	virtual uint32 GetNumberOfTouchControllers() const override;
};


namespace QIYIInput
 {
	//-------------------------------------------------------------------------------------------------
	// FQIYIInput 
	//-------------------------------------------------------------------------------------------------


	class FQIYIInput : public IInputDevice, public FXRMotionControllerBase, public IModuleInterface//, public AActor
	{
	public:

		FQIYIInput(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);

		virtual ~FQIYIInput();

		static void PreInit();

		//IInputDevice overrides
		virtual void Tick(float) override;
		virtual void SendControllerEvents() override;
		virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;
		virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
		virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
		virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;

		// IMotionController overrides
		virtual FName GetMotionControllerDeviceTypeName() const override;
		virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const override;
		virtual ETrackingStatus GetControllerTrackingStatus(const int32 ControllerIndex, const EControllerHand DeviceHand) const override;

		uint32 GetNumberOfTouchControllers() const;

	private:

		void UpdateForceFeedback(const FQIYITouchControllerPair& ControllerPair, const EControllerHand Hand);

		bool OnControllerButtonPressed(const FQIYIButtonState& ButtonState, int32 ControllerId, bool IsRepeat);
		bool OnControllerButtonReleased(const FQIYIButtonState& ButtonState, int32 ControllerId, bool IsRepeat);

		void OnControllerButtonReleasedHome(bool& bButtonPressed, FQIYIButtonState& ButtonState);


	private:

		bool bReleaseHome;

		TSharedPtr< FGenericApplicationMessageHandler > MessageHandler;

		TArray< FQIYITouchControllerPair > ControllerPairs;

		FTimerHandle ControllerHandle;

		static int32 TriggerThreshold;
		static float ButtonRepeatDelay;
	};
}

