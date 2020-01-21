#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"


//-----------------------------------------------------------------------------------------------
enum XboxButtonID
{
	XBOX_BUTTON_ID_INVALID = -1,

	// These are "simple" (literal/physical) binary buttons
	XBOX_BUTTON_ID_A,
	XBOX_BUTTON_ID_B,
	XBOX_BUTTON_ID_X,
	XBOX_BUTTON_ID_Y,
	XBOX_BUTTON_ID_BACK,
	XBOX_BUTTON_ID_START,
	XBOX_BUTTON_ID_LSHOULDER,
	XBOX_BUTTON_ID_RSHOULDER,
	XBOX_BUTTON_ID_LTHUMB,
	XBOX_BUTTON_ID_RTHUMB,
	XBOX_BUTTON_ID_DPAD_RIGHT,
	XBOX_BUTTON_ID_DPAD_UP,
	XBOX_BUTTON_ID_DPAD_LEFT,
	XBOX_BUTTON_ID_DPAD_DOWN,
	
	NUM_XBOX_CONTROLLER_BUTTONS
};


//-----------------------------------------------------------------------------------------------
class XboxController
{
	friend class InputSystem;

public:
	explicit XboxController( int controllerID );
	~XboxController();

	bool					IsConnected() const				{ return m_isConnected; }
	int						GetControllerID() const			{ return m_controllerID; }
	const AnalogJoystick	GetLeftJoyStick() const			{ return m_leftJoyStick; }
	const AnalogJoystick	GetRightJoyStick() const		{ return m_rightJoyStick; }
	float					GetLeftTriggerValue() const		{ return m_leftTriggerValue; }
	float					GetRightTriggerValue() const	{ return m_rightTriggerValue; }
	bool					IsButtonPressed(XboxButtonID buttonID) const;
	bool					WasButtonJustPressed(XboxButtonID buttonID) const;
	bool					WasButtonReleased(XboxButtonID buttonID) const;
	void					SetVibrationLevels(float leftFraction, float rightFraction);

private:
	void Update();
	void Reset();
	void UpdateTrigger( float& triggerValue, unsigned char rawValue );
	void UpdateJoyStick( AnalogJoystick& joystick, short rawX, short rawY );
	void UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag );

private:
	const int		m_controllerID = -1;
	bool			m_isConnected = false;
	KeyButtonState	m_buttonStates[ NUM_XBOX_CONTROLLER_BUTTONS ];
	AnalogJoystick	m_leftJoyStick = AnalogJoystick( 0.3f, 0.9f );
	AnalogJoystick	m_rightJoyStick = AnalogJoystick( 0.3f, 0.9f );
	float			m_leftTriggerValue = 0.0f;
	float			m_rightTriggerValue = 0.0f;
	float			m_leftVibrationLevel = 0.0f;
	float			m_rightVibrationLevel = 0.0f;
};
