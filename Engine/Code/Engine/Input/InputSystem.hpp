#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/Vec2.hpp"

#include <queue>


//-----------------------------------------------------------------------------------------------
constexpr int MAX_XBOX_CONTROLLERS = 4;
constexpr int MAX_KEY_CODES = 256;


//-----------------------------------------------------------------------------------------------
// Key Codes
//
extern const unsigned char KEY_ESC;
extern const unsigned char KEY_ENTER;
extern const unsigned char KEY_SPACEBAR;
extern const unsigned char KEY_BACKSPACE;
extern const unsigned char KEY_DELETE;
extern const unsigned char KEY_SHIFT;
extern const unsigned char KEY_UPARROW;
extern const unsigned char KEY_LEFTARROW;
extern const unsigned char KEY_DOWNARROW;
extern const unsigned char KEY_RIGHTARROW;
extern const unsigned char KEY_HOME;
extern const unsigned char KEY_END;
extern const unsigned char KEY_F1;
extern const unsigned char KEY_F2;
extern const unsigned char KEY_F3;
extern const unsigned char KEY_F4;
extern const unsigned char KEY_F5;
extern const unsigned char KEY_F6;
extern const unsigned char KEY_F7;
extern const unsigned char KEY_F8;
extern const unsigned char KEY_F9;
extern const unsigned char KEY_F10;
extern const unsigned char KEY_F11;
extern const unsigned char KEY_F12;
extern const unsigned char KEY_TILDE;


//-----------------------------------------------------------------------------------------------
class InputSystem
{
public:
	InputSystem();
	~InputSystem();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );

	void PushCharacter( char c );
	bool PopCharacter( char* out ); 

	bool IsKeyPressed( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;
	
	bool ConsumeIsKeyPressed( unsigned char keyCode );
	bool ConsumeWasKeyJustPressed( unsigned char keyCode );
	bool ConsumeWasKeyJustReleased( unsigned char keyCode );

	int ConsumeAllKeyPresses( unsigned char keyCode );
	int ConsumeAllKeyReleases( unsigned char keyCode );


	const XboxController&	GetXboxController( int controllerID );
	void					SetXboxControllerVibrationLevels( int controllerID, float leftFraction, float rightFraction );

	void UpdateMouse();
	const Vec2 GetNormalizedMouseClientPos()																					{ return m_normalizedMouseClientPos; }

private:
	KeyButtonState m_keyStates[MAX_KEY_CODES];
	XboxController m_controllers[ MAX_XBOX_CONTROLLERS ] =
	{
		XboxController( 0 ),
		XboxController( 1 ),
		XboxController( 2 ),
		XboxController( 3 ),
	};

	Vec2 m_normalizedMouseClientPos = Vec2::ZERO;

	std::queue<char> m_characters;
};