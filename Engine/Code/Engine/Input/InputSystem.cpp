#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Math/AABB2.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // must #include Windows.h before WinUser.h
#include <WinUser.h>


//-----------------------------------------------------------------------------------------------
const unsigned char KEY_ESC = VK_ESCAPE;
const unsigned char KEY_ENTER = VK_RETURN;
const unsigned char KEY_SHIFT = VK_SHIFT;
const unsigned char KEY_CTRL = VK_CONTROL;
const unsigned char KEY_ALT = VK_MENU;
const unsigned char KEY_SPACEBAR = VK_SPACE;
const unsigned char KEY_BACKSPACE = VK_BACK;
const unsigned char KEY_DELETE = VK_DELETE;
const unsigned char KEY_UPARROW = VK_UP;
const unsigned char KEY_LEFTARROW = VK_LEFT;
const unsigned char KEY_DOWNARROW = VK_DOWN;
const unsigned char KEY_RIGHTARROW = VK_RIGHT;
const unsigned char KEY_HOME = VK_HOME;
const unsigned char KEY_END = VK_END;
const unsigned char KEY_F1 = VK_F1;
const unsigned char KEY_F2 = VK_F2;
const unsigned char KEY_F3 = VK_F3;
const unsigned char KEY_F4 = VK_F4;
const unsigned char KEY_F5 = VK_F5;
const unsigned char KEY_F6 = VK_F6;
const unsigned char KEY_F7 = VK_F7;
const unsigned char KEY_F8 = VK_F8;
const unsigned char KEY_F9 = VK_F9;
const unsigned char KEY_F10 = VK_F10;
const unsigned char KEY_F11 = VK_F11;
const unsigned char KEY_F12 = VK_F12;
const unsigned char KEY_SEMICOLON = VK_OEM_1;
const unsigned char KEY_TILDE = VK_OEM_3;
const unsigned char KEY_LEFT_BRACKET = VK_OEM_4;
const unsigned char KEY_RIGHT_BRACKET = VK_OEM_6;
const unsigned char KEY_QUOTE = VK_OEM_7;
const unsigned char KEY_COMMA = VK_OEM_COMMA;
const unsigned char KEY_PERIOD = VK_OEM_PERIOD;
const unsigned char KEY_PLUS = VK_OEM_PLUS;
const unsigned char KEY_MINUS = VK_OEM_MINUS;
const unsigned char KEY_TAB = VK_TAB;

const unsigned char MOUSE_LBUTTON = VK_LBUTTON;
const unsigned char MOUSE_RBUTTON = VK_RBUTTON;
const unsigned char MOUSE_MBUTTON = VK_MBUTTON;
const unsigned char CMD_PASTE = '\x16';


//-----------------------------------------------------------------------------------------------
InputSystem::InputSystem()
{
}


//-----------------------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
}


//-----------------------------------------------------------------------------------------------
void InputSystem::Startup( Window* window )
{
	m_window = window;

	// Push default mouse options
	MouseOptions newOptions;
	newOptions.m_cursorMode = CURSOR_ABSOLUTE;
	newOptions.m_isVisible = true;
	newOptions.m_isClipped = false;

	m_mouseOptionsStack.push_back( newOptions );

	UpdateFromMouseOptions( newOptions );
}


//-----------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	UpdateMouse();

	for ( int controllerID = 0; controllerID < MAX_XBOX_CONTROLLERS; ++controllerID )
	{
		m_controllers[controllerID].Update();
	}
}


//-----------------------------------------------------------------------------------------------
void InputSystem::EndFrame()
{
	// Copy current key states as old key states for next frame
	for ( int keyCode = 0; keyCode < MAX_KEY_CODES; ++keyCode)
	{
		KeyButtonState& keyState = m_keyStates[keyCode];
		keyState.UpdateStatus( keyState.IsPressed() );
	}

	m_mouseWheelScrollAmountDelta = 0.f;

	for ( int charNum = 0; charNum < (int)m_characters.size(); ++charNum )
	{
		m_characters.pop();
	}
}


//-----------------------------------------------------------------------------------------------
void InputSystem::Shutdown()
{
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::HandleKeyPressed( unsigned char keyCode )
{
	m_keyStates[keyCode].UpdateStatus( true );
	
	return true;
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::HandleKeyReleased( unsigned char keyCode )
{
	m_keyStates[keyCode].UpdateStatus( false );

	return true;
}


//-----------------------------------------------------------------------------------------------
void InputSystem::PushCharacter( char c )
{
	m_characters.push( c );
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::PopCharacter( char* out )
{
	if ( m_characters.size() > 0 )
	{
		*out = m_characters.front();
		m_characters.pop();
		return true;
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
const XboxController& InputSystem::GetXboxController(int controllerID)
{
	return m_controllers[controllerID];
}


//-----------------------------------------------------------------------------------------------
void InputSystem::SetXboxControllerVibrationLevels( int controllerID, float leftFraction, float rightFraction )
{
	m_controllers[controllerID].SetVibrationLevels( leftFraction, rightFraction );
}


//-----------------------------------------------------------------------------------------------
void InputSystem::UpdateMouse()
{
	switch ( m_currentCursorMode )
	{
		case CURSOR_ABSOLUTE:
		{
			POINT mousePos;
			GetCursorPos( &mousePos );
			ScreenToClient( (HWND)m_window->m_hwnd, &mousePos );
			Vec2 mouseClientPos( (float)mousePos.x, (float)mousePos.y );

			RECT clientRect;
			GetClientRect( (HWND)m_window->m_hwnd, &clientRect );

			AABB2 clientBounds( (float)clientRect.left, (float)clientRect.top, (float)clientRect.right, (float)clientRect.bottom );

			m_normalizedMouseClientPos = clientBounds.GetUVForPoint( mouseClientPos );
			m_normalizedMouseClientPos.y = 1.f - m_normalizedMouseClientPos.y;
		} break;

		case CURSOR_RELATIVE:
		{
			POINT mousePos;
			GetCursorPos( &mousePos );
			ScreenToClient( (HWND)m_window->m_hwnd, &mousePos );
			Vec2 mouseClientPos( (float)mousePos.x, (float)mousePos.y );

			m_mouseMovementDelta = mouseClientPos - m_mousePositionLastFrame;// move back to center

			Vec2 windowCenter = GetCenterOfWindow();
			SetCursorPos( (int)windowCenter.x, (int)windowCenter.y ); 
			
			// one trick to prevent drift
			GetCursorPos( &mousePos );
			ScreenToClient( (HWND)m_window->m_hwnd, &mousePos );
			windowCenter = Vec2( (float)mousePos.x, (float)mousePos.y );

			m_mousePositionLastFrame = windowCenter;
		} break;
	}
}


//-----------------------------------------------------------------------------------------------
void InputSystem::HideSystemCursor()
{
	while ( ShowCursor( false ) >= 0 ) {}
}


//-----------------------------------------------------------------------------------------------
void InputSystem::ShowSystemCursor()
{
	// Force the cursor to be shown
	while ( ShowCursor( true ) < 0 ) {}
}


//-----------------------------------------------------------------------------------------------
void InputSystem::LockSystemCursor()
{
	RECT clientRect;
	GetClientRect( (HWND)m_window->m_hwnd, &clientRect );
	AABB2 clientBounds( (float)clientRect.left, (float)clientRect.top, (float)clientRect.right, (float)clientRect.bottom );

	Vec2 clientCenter = clientBounds.GetCenter();

	POINT windowOffset = POINT();
	ClientToScreen( (HWND)m_window->m_hwnd, &windowOffset );
	clientCenter += Vec2( (float)windowOffset.x, (float)windowOffset.y );

	RECT clientBoundsRect;
	clientBoundsRect.left = clientRect.left + windowOffset.x; 
	clientBoundsRect.top = clientRect.top + windowOffset.y;
	clientBoundsRect.right = clientRect.right + windowOffset.x;
	clientBoundsRect.bottom = clientRect.bottom + windowOffset.y;

	ClipCursor( &clientBoundsRect );
}


//-----------------------------------------------------------------------------------------------
void InputSystem::UnlockSystemCursor()
{
	ClipCursor( nullptr );
}


//-----------------------------------------------------------------------------------------------
void InputSystem::UpdateFromMouseOptions( const MouseOptions& options )
{
	if ( options.m_isClipped )
	{
		LockSystemCursor();
	}
	else
	{
		UnlockSystemCursor();
	}

	if ( options.m_isVisible )
	{
		ShowSystemCursor();
	}
	else
	{
		HideSystemCursor();
	}

	switch ( options.m_cursorMode )
	{
		case CURSOR_ABSOLUTE:
		{
		} break;

		case CURSOR_RELATIVE:
		{
			m_mousePositionLastFrame = GetCenterOfWindow();
			SetCursorPos( (int)m_mousePositionLastFrame.x, (int)m_mousePositionLastFrame.y );
		} break;
	}

	m_currentCursorMode = options.m_cursorMode;
}


//-----------------------------------------------------------------------------------------------
void InputSystem::PushMouseOptions( eCursorMode cursorMode, bool isVisible, bool isClipped )
{
	MouseOptions newOptions;
	newOptions.m_cursorMode = cursorMode;
	newOptions.m_isVisible = isVisible;
	newOptions.m_isClipped = isClipped;

	m_mouseOptionsStack.push_back( newOptions );

	UpdateFromMouseOptions( newOptions );
}


//-----------------------------------------------------------------------------------------------
void InputSystem::PopMouseOptions()
{
	if ( m_mouseOptionsStack.size() > 0 )
	{
		m_mouseOptionsStack.pop_back();
	}

	if ( m_mouseOptionsStack.size() > 0 )
	{
		UpdateFromMouseOptions( m_mouseOptionsStack.back() );
	}
	else
	{
		g_devConsole->PrintString( "Tried to pop too many mouse states.", Rgba8::YELLOW );
	}
}


//-----------------------------------------------------------------------------------------------
const char* InputSystem::GetTextFromClipboard() const
{
	HANDLE h;

	if ( !OpenClipboard( NULL ) )
	{
		g_devConsole->PrintString( "Can't open clipboard", Rgba8::RED );
		return nullptr;
	}

	h = GetClipboardData( CF_TEXT );

	const char* data = (const char*)h;

	CloseClipboard();

	return data;
}


//-----------------------------------------------------------------------------------------------
const Vec2 InputSystem::GetCenterOfWindow()
{
	RECT clientRect;
	GetClientRect( (HWND)m_window->m_hwnd, &clientRect );
	AABB2 clientBounds( (float)clientRect.left, (float)clientRect.top, (float)clientRect.right, (float)clientRect.bottom );

	Vec2 clientCenter = clientBounds.GetCenter();

	POINT windowOffset = POINT();
	ClientToScreen( (HWND)m_window->m_hwnd, &windowOffset );
	clientCenter += Vec2( (float)windowOffset.x, (float)windowOffset.y );
	
	return clientCenter;
}


//-----------------------------------------------------------------------------------------------
void InputSystem::UpdateMouseButtonState( bool leftButtonDown, bool rightButtonDown, bool middleButtonDown )
{
	m_keyStates[MOUSE_LBUTTON].UpdateStatus( leftButtonDown );
	m_keyStates[MOUSE_RBUTTON].UpdateStatus( rightButtonDown );
	m_keyStates[MOUSE_MBUTTON].UpdateStatus( middleButtonDown );
}


//-----------------------------------------------------------------------------------------------
void InputSystem::AddMouseWheelScrollAmount( float scrollAmount )
{
	m_mouseWheelScrollAmountDelta += scrollAmount;
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::IsKeyPressed( unsigned char keyCode ) const
{
	return m_keyStates[keyCode].IsPressed();
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed( unsigned char keyCode ) const
{
	return m_keyStates[keyCode].WasJustPressed();

}


//-----------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustReleased( unsigned char keyCode ) const
{
	return m_keyStates[keyCode].WasJustReleased();
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::WasAnyKeyJustPressed() const
{
	for ( int keyIdx = 0; keyIdx < MAX_KEY_CODES; ++keyIdx )
	{
		if ( m_keyStates[keyIdx].IsPressed() )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::ConsumeAnyKeyJustPressed()
{
	bool wasAnyJustPressed = WasAnyKeyJustPressed();

	ResetAllKeys();

	return wasAnyJustPressed;
}


//-----------------------------------------------------------------------------------------------
bool InputSystem::ConsumeKeyPress( unsigned char keyCode )
{
	return m_keyStates[keyCode].ConsumeKeyPress();
}


//-----------------------------------------------------------------------------------------------
int InputSystem::ConsumeAllKeyPresses( unsigned char keyCode )
{
	return m_keyStates[keyCode].ConsumeAllKeyPresses();
}


//-----------------------------------------------------------------------------------------------
int InputSystem::ConsumeAllKeyReleases( unsigned char keyCode )
{
	return m_keyStates[keyCode].ConsumeAllKeyReleases();
}


//-----------------------------------------------------------------------------------------------
void InputSystem::ResetAllKeys()
{
	for ( int keyCodeIdx = 0; keyCodeIdx < MAX_KEY_CODES; ++keyCodeIdx )
	{
		m_keyStates[keyCodeIdx].Reset();
	}
}
