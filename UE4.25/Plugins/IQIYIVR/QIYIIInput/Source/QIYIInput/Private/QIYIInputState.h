//=============================================================================
//
//                 Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                              All Rights Reserved.
//
//=============================================================================
#pragma once
#include "IQIYIInputModule.h"

#if 1//QIYI_INPUT_SUPPORTED_PLATFORMS
#include "IMotionController.h"
#include "InputCoreTypes.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

namespace QIYIInput
{
	//-------------------------------------------------------------------------------------------------
	// QIYI Button names
	//-------------------------------------------------------------------------------------------------
	
	struct EQIYITouchControllerButton
	{
		enum Type
		{
			Trigger,
			Grip,

			XA,
			YB,
			Thumbstick,

			Thumbstick_Up,
			Thumbstick_Down,
			Thumbstick_Left,
			Thumbstick_Right,

			Menu,

			Thumbstick_Touch,
			XA_Touch,
			YB_Touch,
			Trigger_Touch,
			/** Total number of buttons */
			TotalButtonCount

		};
	};


	//-------------------------------------------------------------------------------------------------
	// FQIYIKey
	//-------------------------------------------------------------------------------------------------

	struct FQIYIKey
	{
		//Button
		static const FKey QIYITouch_Left_Thumbstick_Click;
		static const FKey QIYITouch_Left_Thumbstick_Up;
		static const FKey QIYITouch_Left_Thumbstick_Down;
		static const FKey QIYITouch_Left_Thumbstick_Left;
		static const FKey QIYITouch_Left_Thumbstick_Right;
		static const FKey QIYITouch_Left_Trigger_Click;
		static const FKey QIYITouch_Left_Grip_Click;
		static const FKey QIYITouch_Left_X_Click;
		static const FKey QIYITouch_Left_Y_Click;
		static const FKey QIYITouch_Left_Menu_Click;

		static const FKey QIYITouch_Right_Thumbstick_Click;
		static const FKey QIYITouch_Right_Thumbstick_Up;
		static const FKey QIYITouch_Right_Thumbstick_Down;
		static const FKey QIYITouch_Right_Thumbstick_Left;
		static const FKey QIYITouch_Right_Thumbstick_Right;
		static const FKey QIYITouch_Right_Trigger_Click;
		static const FKey QIYITouch_Right_Grip_Click;
		static const FKey QIYITouch_Right_A_Click;
		static const FKey QIYITouch_Right_B_Click;
		static const FKey QIYITouch_Right_Home_Click;

		//Touch
		static const FKey QIYITouch_Left_Thumbstick_Touch;
		static const FKey QIYITouch_Left_Trigger_Touch;
		static const FKey QIYITouch_Left_X_Touch;
		static const FKey QIYITouch_Left_Y_Touch;

		static const FKey QIYITouch_Right_Thumbstick_Touch;
		static const FKey QIYITouch_Right_Trigger_Touch;
		static const FKey QIYITouch_Right_A_Touch;
		static const FKey QIYITouch_Right_B_Touch;

	};

	//-------------------------------------------------------------------------------------------------
	// FQIYIKeyNames
	//-------------------------------------------------------------------------------------------------

	struct FQIYIKeyNames
	{
		typedef FName Type;

		//Button
		static const FName QIYITouch_Left_Thumbstick_Click;
		static const FName QIYITouch_Left_Thumbstick_Up;
		static const FName QIYITouch_Left_Thumbstick_Down;
		static const FName QIYITouch_Left_Thumbstick_Left;
		static const FName QIYITouch_Left_Thumbstick_Right;
		static const FName QIYITouch_Left_Trigger_Click;
		static const FName QIYITouch_Left_Grip_Click;
		static const FName QIYITouch_Left_X_Click;
		static const FName QIYITouch_Left_Y_Click;
		static const FName QIYITouch_Left_Menu_Click;

		static const FName QIYITouch_Right_Thumbstick_Click;
		static const FName QIYITouch_Right_Thumbstick_Up;
		static const FName QIYITouch_Right_Thumbstick_Down;
		static const FName QIYITouch_Right_Thumbstick_Left;
		static const FName QIYITouch_Right_Thumbstick_Right;
		static const FName QIYITouch_Right_Trigger_Click;
		static const FName QIYITouch_Right_Grip_Click;
		static const FName QIYITouch_Right_A_Click;
		static const FName QIYITouch_Right_B_Click;
		static const FName QIYITouch_Right_Home_Click;

		//Touch
		static const FName QIYITouch_Left_Thumbstick_Touch;
		static const FName QIYITouch_Left_Trigger_Touch;
		static const FName QIYITouch_Left_X_Touch;
		static const FName QIYITouch_Left_Y_Touch;

		static const FName QIYITouch_Right_Thumbstick_Touch;
		static const FName QIYITouch_Right_Trigger_Touch;
		static const FName QIYITouch_Right_A_Touch;
		static const FName QIYITouch_Right_B_Touch;
	};

	//-------------------------------------------------------------------------------------------------
	// FQIYIButtonState -  Digital button state
	//-------------------------------------------------------------------------------------------------

	struct FQIYIButtonState
	{
		FName Key;

		bool bIsPressed;

		double NextRepeatTime;

		FQIYIButtonState()
			: Key(NAME_None),
			bIsPressed(false),
			NextRepeatTime(0.0)
		{
		}
	};

	//-------------------------------------------------------------------------------------------------
	// FQIYITouchControllerState - Input state for an QIYI motion controller
	//-------------------------------------------------------------------------------------------------

	struct FQIYITouchControllerState
	{
	
		bool bIsConnected;
		FVector2D ThumbstickAxes;
		FVector2D TouchpadAxes;
		FQIYIButtonState Buttons_State[(int32)EQIYITouchControllerButton::TotalButtonCount];
		int RecenterCount;

		FQIYITouchControllerState(const EControllerHand Hand)
			: bIsConnected(false),
			ThumbstickAxes(FVector2D::ZeroVector),
			TouchpadAxes(FVector2D::ZeroVector),
			RecenterCount(0)
		{
			for (FQIYIButtonState& Button : Buttons_State)
			{
				Button.bIsPressed = false;
				Button.NextRepeatTime = 0.0;
			}

			//Button
			Buttons_State[(int32)EQIYITouchControllerButton::Menu].Key
					= (Hand == EControllerHand::Left) ? "QIYITouch_Left_Menu_Click" : "QIYITouch_Right_Home_Click";

		    Buttons_State[(int32)EQIYITouchControllerButton::Trigger].Key
					= (Hand == EControllerHand::Left) ? "QIYITouch_Left_Trigger_Click" : "QIYITouch_Right_Trigger_Click";

			Buttons_State[(int32)EQIYITouchControllerButton::Grip].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_Grip_Click" : "QIYITouch_Right_Grip_Click";

			Buttons_State[(int32)EQIYITouchControllerButton::Thumbstick].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_Thumbstick_Click" : "QIYITouch_Right_Thumbstick_Click";

			Buttons_State[(int32)EQIYITouchControllerButton::XA].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_X_Click" : "QIYITouch_Right_A_Click";

			Buttons_State[(int32)EQIYITouchControllerButton::YB].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_Y_Click" : "QIYITouch_Right_B_Click";

			Buttons_State[(int32)EQIYITouchControllerButton::Thumbstick_Up].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_Thumbstick_Up" : "QIYITouch_Right_Thumbstick_Up";

			Buttons_State[(int32)EQIYITouchControllerButton::Thumbstick_Down].Key
					= (Hand == EControllerHand::Left) ? "QIYITouch_Left_Thumbstick_Down" : "QIYITouch_Right_Thumbstick_Down";

			Buttons_State[(int32)EQIYITouchControllerButton::Thumbstick_Left].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_Thumbstick_Left" : "QIYITouch_Right_Thumbstick_Left";

			Buttons_State[(int32)EQIYITouchControllerButton::Thumbstick_Right].Key
					=(Hand == EControllerHand::Left) ? "QIYITouch_Left_Thumbstick_Right" : "QIYITouch_Right_Thumbstick_Right";

			//Touch
			Buttons_State[(int32)EQIYITouchControllerButton::Thumbstick_Touch].Key
				= (Hand == EControllerHand::Left) ? "QIYITouch_Left_Thumbstick_Touch" : "QIYITouch_Right_Thumbstick_Touch";

			Buttons_State[(int32)EQIYITouchControllerButton::Trigger_Touch].Key
				= (Hand == EControllerHand::Left) ? "QIYITouch_Left_Trigger_Touch" : "QIYITouch_Right_Trigger_Touch";

			Buttons_State[(int32)EQIYITouchControllerButton::XA_Touch].Key
				= (Hand == EControllerHand::Left) ? "QIYITouch_Left_X_Touch" : "QIYITouch_Right_A_Touch";

			Buttons_State[(int32)EQIYITouchControllerButton::YB_Touch].Key
				= (Hand == EControllerHand::Left) ? "QIYITouch_Left_Y_Touch" : "QIYITouch_Right_B_Touch";

		}

		FQIYITouchControllerState()
		{
		}

	};

	//-------------------------------------------------------------------------------------------------
	// FQIYITouchControllerPair - A pair of wireless motion controllers, one for either hand
	//-------------------------------------------------------------------------------------------------
	struct FQIYITouchControllerPair
	{

		int32 UnrealControllerIndex;

		FQIYITouchControllerState ControllerStates[2];

		FQIYITouchControllerPair()
			: UnrealControllerIndex(INDEX_NONE),
			ControllerStates()
		{
			ControllerStates[(int32)EControllerHand::Left] = FQIYITouchControllerState(EControllerHand::Left);
			ControllerStates[(int32)EControllerHand::Right] = FQIYITouchControllerState(EControllerHand::Right);
		}
	};
}
#endif