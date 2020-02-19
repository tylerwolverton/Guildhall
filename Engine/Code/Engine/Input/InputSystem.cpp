#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/AABB2.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // must #include Windows.h before WinUser.h
#include <WinUser.h>


//-----------------------------------------------------------------------------------------------
const unsigned char KEY_ESC = VK_ESCAPE;
const unsigned char KEY_ENTER = VK_RETURN;
const unsigned char KEY_LEFT_SHIFT = VK_LSHIFT;
const unsigned char KEY_RIGHT_SHIFT = VK_RSHIFT;
const unsigned char KEY_LEFT_CTRL = VK_LCONTROL;
const unsigned char KEY_RIGHT_CTRL = VK_RCONTROL;
const unsigned char KEY_LEFT_ALT = VK_LMENU;
const unsigned char KEY_RIGHT_ALT = VK_RMENU;
const unsigned char KEY_SPACEBAR = VK_SPACE;
const unsigned char KEY_BACKSPACE = VK_BACK;
const unsigned char KEY_DELETE = VK_DELETE;
const unsigned char KEY_SHIFT = VK_SHIFT;
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
const unsigned char KEY_TILDE = VK_OEM_3;

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
void InputSystem::Startup()
{
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
	/*POINT mousePos;
	GetCursorPos( &mousePos );
	ScreenToClient( g_hWnd, &mousePos );
	Vec2 mouseClientPos( (float)mousePos.x, (float)mousePos.y );

	RECT clientRect;
	GetClientRect( g_hWnd, &clientRect );

	AABB2 clientBounds( (float)clientRect.left, (float)clientRect.top, (float)clientRect.right, (float)clientRect.bottom );
	
	m_normalizedMouseClientPos = clientBounds.GetUVForPoint( mouseClientPos );
	m_normalizedMouseClientPos.y = 1.f - m_normalizedMouseClientPos.y;*/
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
bool InputSystem::ConsumeKeyPress( unsigned char keyCode )
{
	return m_keyStates[keyCode].ConsumeKeyPress();
}


//-----------------------------------------------------------------------------------------------
int InputSystem::ConsumeAllKeyPresses( unsigned char keyCode )
{
	return m_keyStates[keyCode].ConsumeAllKeyPresses();
}
