//=============================================================================
//
//                 Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                              All Rights Reserved.
//
//=============================================================================

#include "QIYIInput.h"
#include "Misc/CoreDelegates.h"
#include "Features/IModularFeatures.h"
#include "Misc/ConfigCacheIni.h"
#include "Internationalization/Internationalization.h"
#include "IQIYIVRHMDModule.h"
#include "QIYIVRSDKCore.h"
#include "Math/UnrealMathUtility.h"
#include "sxrApi.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "QIYIInput"

void FQIYIInputModule::StartupModule()
{
	IInputDeviceModule::StartupModule();
	QIYIInput::FQIYIInput::PreInit();

	UE_LOG(LogTemp, Warning, TEXT("QIYIInput::Startup Module"));
}

	
TSharedPtr< class IInputDevice > FQIYIInputModule::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	
  	TSharedPtr< QIYIInput::FQIYIInput > InputDevice(new QIYIInput::FQIYIInput(InMessageHandler));
  	QIYIInputDevice = InputDevice;

	UE_LOG(LogTemp, Warning, TEXT("QIYIInput::Create Input Device "));
	return InputDevice;
}

uint32 FQIYIInputModule::GetNumberOfTouchControllers() const
{
 	if (QIYIInputDevice.IsValid())
 	{
 		return QIYIInputDevice.Pin()->GetNumberOfTouchControllers();
 	}
 
 	return 0;
}
IMPLEMENT_MODULE(FQIYIInputModule, QIYIInput)




namespace QIYIInput
{
	//Button
	const FKey FQIYIKey::QIYITouch_Left_Thumbstick_Click("QIYITouch_Left_Thumbstick_Click");
	const FKey FQIYIKey::QIYITouch_Left_Thumbstick_Up("QIYITouch_Left_Thumbstick_Up");
	const FKey FQIYIKey::QIYITouch_Left_Thumbstick_Down("QIYITouch_Left_Thumbstick_Down");
	const FKey FQIYIKey::QIYITouch_Left_Thumbstick_Left("QIYITouch_Left_Thumbstick_Left");
	const FKey FQIYIKey::QIYITouch_Left_Thumbstick_Right("QIYITouch_Left_Thumbstick_Right");
	const FKey FQIYIKey::QIYITouch_Left_Trigger_Click("QIYITouch_Left_Trigger_Click");
	const FKey FQIYIKey::QIYITouch_Left_Grip_Click("QIYITouch_Left_Grip_Click");
	const FKey FQIYIKey::QIYITouch_Left_X_Click("QIYITouch_Left_X_Click");
	const FKey FQIYIKey::QIYITouch_Left_Y_Click("QIYITouch_Left_Y_Click");
	const FKey FQIYIKey::QIYITouch_Left_Menu_Click("QIYITouch_Left_Menu_Click");

	const FKey FQIYIKey::QIYITouch_Right_Thumbstick_Click("QIYITouch_Right_Thumbstick_Click");
	const FKey FQIYIKey::QIYITouch_Right_Thumbstick_Up("QIYITouch_Right_Thumbstick_Up");
	const FKey FQIYIKey::QIYITouch_Right_Thumbstick_Down("QIYITouch_Right_Thumbstick_Down");
	const FKey FQIYIKey::QIYITouch_Right_Thumbstick_Left("QIYITouch_Right_Thumbstick_Left");
	const FKey FQIYIKey::QIYITouch_Right_Thumbstick_Right("QIYITouch_Right_Thumbstick_Right");
	const FKey FQIYIKey::QIYITouch_Right_Trigger_Click("QIYITouch_Right_Trigger_Click");
	const FKey FQIYIKey::QIYITouch_Right_Grip_Click("QIYITouch_Right_Grip_Click");
	const FKey FQIYIKey::QIYITouch_Right_A_Click("QIYITouch_Right_A_Click");
	const FKey FQIYIKey::QIYITouch_Right_B_Click("QIYITouch_Right_B_Click");
	const FKey FQIYIKey::QIYITouch_Right_Home_Click("QIYITouch_Right_Home_Click");


	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Thumbstick_Up("QIYITouch_Left_Thumbstick_Up");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Thumbstick_Down("QIYITouch_Left_Thumbstick_Down");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Thumbstick_Left("QIYITouch_Left_Thumbstick_Left");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Thumbstick_Right("QIYITouch_Left_Thumbstick_Right");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Trigger_Click("QIYITouch_Left_Trigger_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Grip_Click("QIYITouch_Left_Grip_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_X_Click("QIYITouch_Left_X_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Y_Click("QIYITouch_Left_Y_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Menu_Click("QIYITouch_Left_Menu_Click");

	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Thumbstick_Click("QIYITouch_Left_Thumbstick_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Thumbstick_Up("QIYITouch_Right_Thumbstick_Up");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Thumbstick_Down("QIYITouch_Right_Thumbstick_Down");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Thumbstick_Left("QIYITouch_Right_Thumbstick_Left");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Thumbstick_Right("QIYITouch_Right_Thumbstick_Right");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Trigger_Click("QIYITouch_Right_Trigger_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Grip_Click("QIYITouch_Right_Grip_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_A_Click("QIYITouch_Right_A_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_B_Click("QIYITouch_Right_B_Click");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Home_Click("QIYITouch_Right_Home_Click");


	//Touch
	const FKey FQIYIKey::QIYITouch_Left_Thumbstick_Touch("QIYITouch_Left_Thumbstick_Touch");
	const FKey FQIYIKey::QIYITouch_Left_Trigger_Touch("QIYITouch_Left_Trigger_Touch");
	const FKey FQIYIKey::QIYITouch_Left_X_Touch("QIYITouch_Left_X_Touch");
	const FKey FQIYIKey::QIYITouch_Left_Y_Touch("QIYITouch_Left_Y_Touch");
	const FKey FQIYIKey::QIYITouch_Right_Thumbstick_Touch("QIYITouch_Right_Thumbstick_Touch");
	const FKey FQIYIKey::QIYITouch_Right_Trigger_Touch("QIYITouch_Right_Trigger_Touch");
	const FKey FQIYIKey::QIYITouch_Right_A_Touch("QIYITouch_Right_A_Touch");
	const FKey FQIYIKey::QIYITouch_Right_B_Touch("QIYITouch_Right_B_Touch");

	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Thumbstick_Touch("QIYITouch_Left_Thumbstick_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Trigger_Touch("QIYITouch_Left_Trigger_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_X_Touch("QIYITouch_Left_X_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Left_Y_Touch("QIYITouch_Left_Y_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Thumbstick_Touch("QIYITouch_Right_Thumbstick_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_Trigger_Touch("QIYITouch_Right_Trigger_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_A_Touch("QIYITouch_Right_A_Touch");
	const FQIYIKeyNames::Type FQIYIKeyNames::QIYITouch_Right_B_Touch("QIYITouch_Right_B_Touch");

	int32 FQIYIInput::TriggerThreshold = 5; 

	float FQIYIInput::ButtonRepeatDelay = 0.1f;			


	FQIYIInput::FQIYIInput(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
		: MessageHandler(InMessageHandler)
		, ControllerPairs()
		, bReleaseHome(false)
	{

		FQIYITouchControllerPair& ControllerPair = *new(ControllerPairs) FQIYITouchControllerPair();

		ControllerPair.UnrealControllerIndex = 0; 

		IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);

		UE_LOG(LogTemp, Warning, TEXT("QIYIInput::Init"));
	}


	FQIYIInput::~FQIYIInput()
	{
	}

	void FQIYIInput::PreInit()
	{

		// Register the FKeys
		//Button
		EKeys::AddMenuCategoryDisplayInfo("QIYITouch", FText::FromString("QIYITouch"), TEXT("GraphEditor.PadEvent_16x"));

		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Thumbstick_Up,      LOCTEXT("QIYITouch_Left_Thumbstick_Up",     "QIYI Touch (L) Thumbstick Up"),      FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Thumbstick_Down,    LOCTEXT("QIYITouch_Left_Thumbstick_Down",   "QIYI Touch (L) Thumbstick Down"),    FKeyDetails::GamepadKey, "QIYITouch"));
 		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Thumbstick_Left,    LOCTEXT("QIYITouch_Left_Thumbstick_Left",   "QIYI Touch (L) Thumbstick Left"),    FKeyDetails::GamepadKey, "QIYITouch"));
 		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Thumbstick_Right,   LOCTEXT("QIYITouch_Left_Thumbstick_Right",  "QIYI Touch (L) Thumbstick Right"),   FKeyDetails::GamepadKey, "QIYITouch"));
 		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Trigger_Click,      LOCTEXT("QIYITouch_Left_Trigger_Click",     "QIYI Touch (L) Trigger"),            FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Grip_Click,         LOCTEXT("QIYITouch_Left_Grip_Click",        "QIYI Touch (L) Grip"),               FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_X_Click,            LOCTEXT("QIYITouch_Left_X_Click",           "QIYI Touch (L) X Press"),            FKeyDetails::GamepadKey, "QIYITouch"));
 		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Y_Click,            LOCTEXT("QIYITouch_Left_Y_Click",           "QIYI Touch (L) Y Press"),            FKeyDetails::GamepadKey, "QIYITouch"));
 		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Menu_Click,         LOCTEXT("QIYITouch_Left_Menu_Click",        "QIYI Touch (L) Menu"),               FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Thumbstick_Click,   LOCTEXT("QIYITouch_Left_Thumbstick_Click",  "QIYI Touch (L) Thumbstick"),         FKeyDetails::GamepadKey, "QIYITouch"));
		
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Thumbstick_Up,     LOCTEXT("QIYITouch_Right_Thumbstick_Up",    "QIYI Touch (R) Thumbstick Up"),      FKeyDetails::GamepadKey, "QIYITouch"));
 		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Thumbstick_Down,   LOCTEXT("QIYITouch_Right_Thumbstick_Down",  "QIYI Touch (R) Thumbstick Down"),    FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Thumbstick_Left,   LOCTEXT("QIYITouch_Right_Thumbstick_Left",  "QIYI Touch (R) Thumbstick Left"),    FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Thumbstick_Right,  LOCTEXT("QIYITouch_Right_Thumbstick_Right", "QIYI Touch (R) Thumbstick Right"),   FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Trigger_Click,     LOCTEXT("QIYITouch_Right_Trigger_Click",    "QIYI Touch (R) Trigger"),            FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Grip_Click,		   LOCTEXT("QIYITouch_Right_Grip_Click",	   "QIYI Touch (R) Grip"),               FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_A_Click,           LOCTEXT("QIYITouch_Right_A_Click",          "QIYI Touch (R) A Press"),            FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_B_Click,           LOCTEXT("QIYITouch_Right_B_Click",          "QIYI Touch (R) B Press"),            FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Home_Click,        LOCTEXT("QIYITouch_Right_Home_Click",       "QIYI Touch (R) Home"),               FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Thumbstick_Click,  LOCTEXT("QIYITouch_Right_Thumbstick_Click", "QIYI Touch (R) Thumbstick"),		 FKeyDetails::GamepadKey, "QIYITouch"));

		//Touch
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Thumbstick_Touch,   LOCTEXT("QIYITouch_Left_Thumbstick_Touch",  "QIYI Touch (L) Thumbstick Touch"),   FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Trigger_Touch,	   LOCTEXT("QIYITouch_Left_Trigger_Touch",     "QIYI Touch (L) Trigger Touch"),      FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_X_Touch,            LOCTEXT("QIYITouch_Left_X_Touch",           "QIYI Touch (L) X Touch"),            FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Left_Y_Touch,            LOCTEXT("QIYITouch_Left_Y_Touch",           "QIYI Touch (L) Y Touch"),            FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Thumbstick_Touch,  LOCTEXT("QIYITouch_Right_Thumbstick_Touch", "QIYI Touch (R) Thumbstick Touch"),   FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_Trigger_Touch,     LOCTEXT("QIYITouch_Right_Trigger_Touch",    "QIYI Touch (R) Trigger Touch"),      FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_A_Touch,           LOCTEXT("QIYITouch_Right_A_Touch",          "QIYI Touch (R)  A Touch"),           FKeyDetails::GamepadKey, "QIYITouch"));
		EKeys::AddKey(FKeyDetails(FQIYIKey::QIYITouch_Right_B_Touch,           LOCTEXT("QIYITouch_Right_B_Touch",          "QIYI Touch (R)  B Touch"),           FKeyDetails::GamepadKey, "QIYITouch"));



		UE_LOG(LogTemp, Warning, TEXT(" Register the FKeys"));
	}

	void FQIYIInput::Tick(float DeltaTime)
	{
		// Nothing to do when ticking, for now.  SendControllerEvents() handles everything.
	}

	void FQIYIInput::SendControllerEvents()
	{
		const double CurrentTime = FPlatformTime::Seconds();

#if QIYIVR_SUPPORTED_PLATFORMS	

		if (MessageHandler.IsValid())
		{
			QiyiVRCore::ControllerData LeftData;
			QiyiVRCore::ControllerData RightData;
			QiyiVRCore::QVR_GetControllerDate(&LeftData, &RightData);
			for (FQIYITouchControllerPair& ControllerPair : ControllerPairs)
			{
				for (int32 HandIndex = 0; HandIndex < UE_ARRAY_COUNT(ControllerPair.ControllerStates); ++HandIndex)
				{
					FQIYITouchControllerState& State = ControllerPair.ControllerStates[HandIndex];
					bool bIsLeft = (HandIndex == (int32)EControllerHand::Left);
					QiyiVRCore::ControllerData TempData = LeftData;
	
					if (!bIsLeft)
					{
						TempData = RightData;
					}

					bool isThumbstickTouchState = bIsLeft ? ((int)QiyiVRCore::ENUM_BUTTON::JoyStick & TempData.buttonTouch) != 0 : ((int)QiyiVRCore::ENUM_BUTTON::JoyStick & TempData.buttonTouch) != 0;

					FVector2D TempVector(TempData.joyStickPos.x, TempData.joyStickPos.y);
		
					for (int32 ButtonIndex = 0; ButtonIndex < (int32)EQIYITouchControllerButton::Type::TotalButtonCount; ++ButtonIndex)
					{
						FQIYIButtonState& ButtonState = State.Buttons_State[ButtonIndex];
						check(!ButtonState.Key.IsNone());

						bool bButtonPressed = false;
						switch ((EQIYITouchControllerButton::Type)ButtonIndex)
						{
						case EQIYITouchControllerButton::Type::Trigger:
							bButtonPressed = bIsLeft ? LeftData.triggerForce >= TriggerThreshold : RightData.triggerForce >= TriggerThreshold;
							break;

						case EQIYITouchControllerButton::Type::Grip:
							bButtonPressed = TempData.gripForce >= TriggerThreshold;
							bButtonPressed = bIsLeft ? LeftData.gripForce >= TriggerThreshold : RightData.gripForce >= TriggerThreshold;
							break;

						case EQIYITouchControllerButton::Type::XA:
							bButtonPressed = bIsLeft ? (int)QiyiVRCore::ENUM_BUTTON::A & LeftData.button : (int)QiyiVRCore::ENUM_BUTTON::A & RightData.button;
							break;

						case EQIYITouchControllerButton::Type::YB:
							bButtonPressed = bIsLeft ? (int)QiyiVRCore::ENUM_BUTTON::B & LeftData.button : (int)QiyiVRCore::ENUM_BUTTON::B & RightData.button;
							break;

						case EQIYITouchControllerButton::Type::Thumbstick:
							bButtonPressed = bIsLeft ? (int)QiyiVRCore::ENUM_BUTTON::JoyStick & LeftData.button : (int)QiyiVRCore::ENUM_BUTTON::JoyStick & RightData.button;
							break;

						case EQIYITouchControllerButton::Type::Thumbstick_Up:
							if(TempVector.Size() > 0.5f  && State.Buttons_State[(int)EQIYITouchControllerButton::Thumbstick].bIsPressed
								|| TempVector.Size() > 0.7f && isThumbstickTouchState)
							{
								float Angle = FMath::Atan2(TempVector.Y, TempVector.X);
								bButtonPressed = Angle >= (1.0f / 4.0f) * PI && Angle <= (3.0f / 4.0f) * PI;
							}
							break;
						case EQIYITouchControllerButton::Type::Thumbstick_Down:
							if (TempVector.Size() > 0.5f && State.Buttons_State[(int)EQIYITouchControllerButton::Thumbstick].bIsPressed
								|| TempVector.Size() > 0.7f && isThumbstickTouchState) 
							{
								float Angle = FMath::Atan2(TempVector.Y, TempVector.X);
								bButtonPressed = Angle >= (-3.0f / 4.0f) * PI && Angle <= (-1.0f / 4.0f) * PI;
							}
							break;

						case EQIYITouchControllerButton::Type::Thumbstick_Left:
							if (TempVector.Size() > 0.5f && State.Buttons_State[(int)EQIYITouchControllerButton::Thumbstick].bIsPressed
								|| TempVector.Size() > 0.7f && isThumbstickTouchState) 
							{
 								float Angle = FMath::Atan2(TempVector.Y, TempVector.X);
 								bButtonPressed = Angle <= (-3.0f / 4.0f) * PI || Angle >= (3.0f / 4.0f) * PI;
 							}
							break;

						case EQIYITouchControllerButton::Type::Thumbstick_Right:
							if (TempVector.Size() > 0.5f && State.Buttons_State[(int)EQIYITouchControllerButton::Thumbstick].bIsPressed
								|| TempVector.Size() > 0.7f && isThumbstickTouchState) 
							{
 								float Angle = FMath::Atan2(TempVector.Y, TempVector.X);
 								bButtonPressed = Angle >= (-1.0f / 4.0f) * PI && Angle <= (1.0f / 4.0f) * PI;
 							}
							break;

						case EQIYITouchControllerButton::Type::Menu:
							bButtonPressed = (int)QiyiVRCore::ENUM_BUTTON::Home & TempData.button;
							if (!bIsLeft)
							{
								OnControllerButtonReleasedHome(bButtonPressed, ButtonState);
							}
							break;
							
						case EQIYITouchControllerButton::Type::Thumbstick_Touch:
							bButtonPressed = bIsLeft ? (int)QiyiVRCore::ENUM_BUTTON::JoyStick & LeftData.buttonTouch : (int)QiyiVRCore::ENUM_BUTTON::JoyStick & RightData.buttonTouch;
							break;

						case EQIYITouchControllerButton::Type::Trigger_Touch:
							bButtonPressed = (int)QiyiVRCore::ENUM_BUTTON::Trigger & TempData.buttonTouch;
							break;

						case EQIYITouchControllerButton::Type::XA_Touch:
							bButtonPressed = (int)QiyiVRCore::ENUM_BUTTON::A & TempData.buttonTouch;
							break;

						case EQIYITouchControllerButton::Type::YB_Touch:
							bButtonPressed = (int)QiyiVRCore::ENUM_BUTTON::B & TempData.buttonTouch;
							break;
							

						default:
							check(0);
							break;
						}

						if (bButtonPressed != ButtonState.bIsPressed)
						{
							ButtonState.bIsPressed = bButtonPressed;
							if (ButtonState.bIsPressed)
							{
								OnControllerButtonPressed(ButtonState, ControllerPair.UnrealControllerIndex, false);

								ButtonState.NextRepeatTime = CurrentTime + ButtonRepeatDelay;
							}
							else
							{
								OnControllerButtonReleased(ButtonState, ControllerPair.UnrealControllerIndex, false);
							}

						}

						if (ButtonState.bIsPressed && ButtonState.NextRepeatTime <= CurrentTime)
						{
							OnControllerButtonPressed(ButtonState, ControllerPair.UnrealControllerIndex, true);

							ButtonState.NextRepeatTime = CurrentTime + ButtonRepeatDelay;
						}
					}

				}
			}
		}
#endif
		

	}

	void FQIYIInput::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
	{
		MessageHandler = InMessageHandler;
		UE_LOG(LogTemp, Warning, TEXT("QIYIInput::Set Message Handler"));
	}

	bool FQIYIInput::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
	{
		// No exec commands supported, for now.
		return false;
	}

	void FQIYIInput::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
	{
	}

	void FQIYIInput::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values)
	{
	}

	void FQIYIInput::UpdateForceFeedback(const FQIYITouchControllerPair& ControllerPair, const EControllerHand Hand)
	{
	}

	bool FQIYIInput::OnControllerButtonPressed(const FQIYIButtonState& ButtonState, int32 ControllerId, bool IsRepeat)
	{
		bool result = MessageHandler->OnControllerButtonPressed(ButtonState.Key, ControllerId, IsRepeat);

		return result;
	}

	bool FQIYIInput::OnControllerButtonReleased(const FQIYIButtonState& ButtonState, int32 ControllerId, bool IsRepeat)
	{
		bool result = MessageHandler->OnControllerButtonReleased(ButtonState.Key, ControllerId, IsRepeat);

		return result;
	}

	void FQIYIInput::OnControllerButtonReleasedHome(bool& bButtonPressed, FQIYIButtonState& ButtonState)
	{

#if QIYIVR_SUPPORTED_PLATFORMS	
		if ((bButtonPressed != ButtonState.bIsPressed) && !bButtonPressed )
		{
			if (bReleaseHome)
			{
				bReleaseHome = false;
				GEngine->GetWorldContexts()[0].World()->GetTimerManager().ClearTimer(ControllerHandle);
				return;
			}
			else
			{
				GEngine->GetWorldContexts()[0].World()->GetTimerManager().ClearTimer(ControllerHandle);
				QiyiVRCore::QVR_SendHomeEvent();
			}

		}

		if ((bButtonPressed == ButtonState.bIsPressed) && (bButtonPressed == true) && !ControllerHandle.IsValid())
		{
			auto lambda = [this]() {
				this->bReleaseHome = true;

				sxrRecenterPose();
			};
			GEngine->GetWorldContexts()[0].World()->GetTimerManager().SetTimer(ControllerHandle, lambda, 2, false);
	}
#endif
	}

	FName FQIYIInput::GetMotionControllerDeviceTypeName() const
	{
		const static FName DefaultName(TEXT("QIYIInputDevice"));
		return DefaultName;
	}

	bool FQIYIInput::GetControllerOrientationAndPosition(const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
	{
		
#if QIYIVR_SUPPORTED_PLATFORMS
		if (IsInGameThread())
		{
			for (const FQIYITouchControllerPair& ControllerPair : ControllerPairs)
			{
				if (ControllerPair.UnrealControllerIndex == ControllerIndex)
				{
					if ((DeviceHand == EControllerHand::Left) || (DeviceHand == EControllerHand::Right))
					{
						
						QiyiVRCore::ControllerData LeftData;
						QiyiVRCore::ControllerData RightData;
						QiyiVRCore::QVR_GetControllerDate(&LeftData, &RightData);

						QiyiVRCore::ControllerData TempData = DeviceHand == EControllerHand::Left ? LeftData : RightData;

						FVector TempVector(TempData.position.x * WorldToMetersScale, TempData.position.y * WorldToMetersScale, TempData.position.z * WorldToMetersScale);
						OutPosition = TempVector;

						FQuat TempQuat(TempData.rotation.x, TempData.rotation.y, TempData.rotation.z, TempData.rotation.w);
						OutOrientation = TempQuat.Rotator();

						return true;
							
					}
				}
				break;

			}
		}
			
		

#endif
		return false;
	}

	ETrackingStatus FQIYIInput::GetControllerTrackingStatus(const int32 ControllerIndex, const EControllerHand DeviceHand) const
	{
		ETrackingStatus TrackingStatus = ETrackingStatus::NotTracked;

		return TrackingStatus;
	}

	uint32 FQIYIInput::GetNumberOfTouchControllers() const
	{
		uint32 RetVal = 0;

		for (FQIYITouchControllerPair Pair : ControllerPairs)
		{
			RetVal += (Pair.ControllerStates[0].bIsConnected ? 1 : 0);
			RetVal += (Pair.ControllerStates[1].bIsConnected ? 1 : 0);
		}

		return RetVal;
	}

}

#undef LOCTEXT_NAMESPACE