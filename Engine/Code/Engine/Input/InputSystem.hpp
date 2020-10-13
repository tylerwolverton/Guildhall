#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputCommon.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/Vec2.hpp"

#include <queue>


//-----------------------------------------------------------------------------------------------
class Window;


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

	// Accessors
	const KeyButtonState* GetKeyStates() const												{ return m_keyStates; }

	// Keyboard
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool IsKeyPressed( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;

	bool WasAnyKeyJustPressed() const;
	bool ConsumeAnyKeyJustPressed();
	
	bool ConsumeKeyPress( unsigned char keyCode );
	int ConsumeAllKeyPresses( unsigned char keyCode );
	void ResetAllKeys();

	void PushCharacter( char c );
	bool PopCharacter( char* out );

	// Controller
	const XboxController&	GetXboxController( int controllerID );
	void					SetXboxControllerVibrationLevels( int controllerID, float leftFraction, float rightFraction );

	// Mouse
	void UpdateMouse();
	void UpdateMouseButtonState( bool leftButtonDown, bool rightButtonDown, bool middleButtonDown );
	void AddMouseWheelScrollAmount( float scrollAmount );
	float GetMouseWheelScrollAmountDelta() const									{ return m_mouseWheelScrollAmountDelta; }
	const Vec2 GetNormalizedMouseClientPos()																					{ return m_normalizedMouseClientPos; }
	const Vec2 GetMouseDeltaPosition()																							{ return m_mouseMovementDelta; }
	void HideSystemCursor();
	void ShowSystemCursor();
	void LockSystemCursor();
	void UnlockSystemCursor();
	void UpdateFromMouseOptions( const MouseOptions& options );
	eCursorMode GetCursorMode()																									{ return m_currentCursorMode; }

	void PushMouseOptions( eCursorMode cursorMode, bool isVisible, bool isClipped );
	void PopMouseOptions();

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
	float m_mouseWheelScrollAmountDelta = 0.f;

	eCursorMode m_currentCursorMode = CURSOR_ABSOLUTE;
	eCursorMode m_lastCursorMode = CURSOR_ABSOLUTE;
	Vec2 m_mousePositionLastFrame = Vec2::ZERO;
	Vec2 m_mouseMovementDelta = Vec2::ZERO;

	std::queue<char> m_characters;

	std::vector<MouseOptions> m_mouseOptionsStack;
};