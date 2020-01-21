#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // must #include Windows.h before #including Xinput.h
#include <Xinput.h> // include the Xinput API header file (interface)
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use version 9_1_0 explicitly for broadest compatibility


//-----------------------------------------------------------------------------------------------
XboxController::XboxController( int controllerID )
	: m_controllerID( controllerID )
{
}


//-----------------------------------------------------------------------------------------------
XboxController::~XboxController()
{
}


//-----------------------------------------------------------------------------------------------
bool XboxController::IsButtonPressed( XboxButtonID buttonID ) const
{
	return m_buttonStates[buttonID].IsPressed();
}


//-----------------------------------------------------------------------------------------------
bool XboxController::WasButtonJustPressed( XboxButtonID buttonID ) const
{
	return m_buttonStates[buttonID].WasJustPressed();
}


//-----------------------------------------------------------------------------------------------
bool XboxController::WasButtonReleased( XboxButtonID buttonID ) const
{
	return m_buttonStates[buttonID].WasJustReleased();
}


//-----------------------------------------------------------------------------------------------
void XboxController::SetVibrationLevels( float leftFraction, float rightFraction )
{
	constexpr float maxVibrationValue = 65535.f;

	m_leftVibrationLevel = leftFraction * maxVibrationValue;
	m_rightVibrationLevel = rightFraction * maxVibrationValue;
}


//-----------------------------------------------------------------------------------------------
void XboxController::Update()
{
	XINPUT_STATE xboxControllerState;
	// Default bools to false so a memset can be used to clear it all
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );
	if ( errorStatus == ERROR_SUCCESS )
	{
		m_isConnected = true;
		XINPUT_GAMEPAD& gamepad = xboxControllerState.Gamepad;
		// Update all joysticks and buttons
		UpdateJoyStick( m_leftJoyStick, gamepad.sThumbLX, gamepad.sThumbLY );
		UpdateJoyStick( m_rightJoyStick, gamepad.sThumbRX, gamepad.sThumbRY );
		UpdateTrigger( m_leftTriggerValue, gamepad.bLeftTrigger );
		UpdateTrigger( m_rightTriggerValue, gamepad.bRightTrigger );
		UpdateButton( XBOX_BUTTON_ID_A, gamepad.wButtons, XINPUT_GAMEPAD_A );
		UpdateButton( XBOX_BUTTON_ID_B, gamepad.wButtons, XINPUT_GAMEPAD_B );
		UpdateButton( XBOX_BUTTON_ID_X, gamepad.wButtons, XINPUT_GAMEPAD_X );
		UpdateButton( XBOX_BUTTON_ID_Y, gamepad.wButtons, XINPUT_GAMEPAD_Y );
		UpdateButton( XBOX_BUTTON_ID_BACK, gamepad.wButtons, XINPUT_GAMEPAD_BACK );
		UpdateButton( XBOX_BUTTON_ID_START, gamepad.wButtons, XINPUT_GAMEPAD_START );
		UpdateButton( XBOX_BUTTON_ID_LSHOULDER, gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
		UpdateButton( XBOX_BUTTON_ID_RSHOULDER, gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
		UpdateButton( XBOX_BUTTON_ID_LTHUMB, gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB );
		UpdateButton( XBOX_BUTTON_ID_RTHUMB, gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB );
		UpdateButton( XBOX_BUTTON_ID_DPAD_RIGHT, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT );
		UpdateButton( XBOX_BUTTON_ID_DPAD_UP, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP );
		UpdateButton( XBOX_BUTTON_ID_DPAD_LEFT, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT );
		UpdateButton( XBOX_BUTTON_ID_DPAD_DOWN, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN );

		// Vibration
		XINPUT_VIBRATION vibrationInfo;
		memset( &vibrationInfo, 0, sizeof( vibrationInfo ) );
		vibrationInfo.wLeftMotorSpeed = (WORD)m_leftVibrationLevel;
		vibrationInfo.wRightMotorSpeed = (WORD)m_rightVibrationLevel;
		XInputSetState( 0, &vibrationInfo );
	}
	else if ( errorStatus == ERROR_DEVICE_NOT_CONNECTED )
	{
		Reset();
		return;
	}
	else
	{
		ERROR_AND_DIE( Stringf("Xbox controller %i sent fatal error.", m_controllerID) );
	}
}


//-----------------------------------------------------------------------------------------------
void XboxController::Reset()
{
	m_isConnected = false;

	for (int buttonIndex = 0; buttonIndex < NUM_XBOX_CONTROLLER_BUTTONS; ++buttonIndex )
	{
		m_buttonStates[buttonIndex].Reset();
	}

	m_leftJoyStick.Reset();
	m_rightJoyStick.Reset();
	m_leftTriggerValue = 0.f;
	m_rightTriggerValue = 0.f;
}


//-----------------------------------------------------------------------------------------------
void XboxController::UpdateTrigger( float& triggerValue, unsigned char rawValue )
{
	triggerValue = ( 1.f / 255.f ) * rawValue;
}


//-----------------------------------------------------------------------------------------------
void XboxController::UpdateJoyStick( AnalogJoystick& joystick, short rawX, short rawY )
{
	float normalizedX = RangeMapFloat( -32768.f, 32767.f, -1.f, 1.f, rawX );
	float normalizedY = RangeMapFloat( -32768.f, 32767.f, -1.f, 1.f, rawY );

	joystick.UpdatePosition( normalizedX, normalizedY );
}


//-----------------------------------------------------------------------------------------------
void XboxController::UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag )
{
	bool isPressed = ( buttonFlags & buttonFlag ) == buttonFlag;
	m_buttonStates[buttonID].UpdateStatus( isPressed );
}
