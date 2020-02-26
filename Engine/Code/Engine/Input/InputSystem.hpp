#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/Vec2.hpp"

#include <queue>


//-----------------------------------------------------------------------------------------------
class Window;


//-----------------------------------------------------------------------------------------------
constexpr int MAX_XBOX_CONTROLLERS = 4;
constexpr int MAX_KEY_CODES = 256;


//-----------------------------------------------------------------------------------------------
// Key Codes
//
extern const unsigned char KEY_ESC;
extern const unsigned char KEY_ENTER;
extern const unsigned char KEY_SHIFT;
extern const unsigned char KEY_CTRL;
extern const unsigned char KEY_ALT;
extern const unsigned char KEY_SPACEBAR;
extern const unsigned char KEY_BACKSPACE;
extern const unsigned char KEY_DELETE;
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

extern const unsigned char CMD_PASTE;


//-----------------------------------------------------------------------------------------------
enum eCursorMode
{
	CURSOR_ABSOLUTE,
	CURSOR_RELATIVE
};


//-----------------------------------------------------------------------------------------------
class InputSystem
{
public:
	InputSystem();
	~InputSystem();

	void Startup( Window* window );
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	// Keyboard
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool IsKeyPressed( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;
	
	bool ConsumeKeyPress( unsigned char keyCode );
	int ConsumeAllKeyPresses( unsigned char keyCode );

	void PushCharacter( char c );
	bool PopCharacter( char* out );

	// Controller
	const XboxController&	GetXboxController( int controllerID );
	void					SetXboxControllerVibrationLevels( int controllerID, float leftFraction, float rightFraction );

	// Mouse
	void UpdateMouse();
	const Vec2 GetNormalizedMouseClientPos()																					{ return m_normalizedMouseClientPos; }
	const Vec2 GetMouseDeltaPosition()																							{ return m_mouseMovementDelta; }
	void HideSystemCursor();
	void ShowSystemCursor();
	void LockSystemCursor();
	void UnlockSystemCursor();
	void SetCursorMode( eCursorMode cursorMode );

	const char* GetTextFromClipboard() const;

private:
	const Vec2 GetCenterOfWindow();

private:
	KeyButtonState m_keyStates[MAX_KEY_CODES];
	XboxController m_controllers[ MAX_XBOX_CONTROLLERS ] =
	{
		XboxController( 0 ),
		XboxController( 1 ),
		XboxController( 2 ),
		XboxController( 3 ),
	};

	Window* m_window = nullptr;
	Vec2 m_normalizedMouseClientPos = Vec2::ZERO;
	eCursorMode m_cursorMode = CURSOR_ABSOLUTE;
	Vec2 m_mousePositionLastFrame = Vec2::ZERO;
	Vec2 m_mouseMovementDelta = Vec2::ZERO;

	std::queue<char> m_characters;
};