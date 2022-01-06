#include "Engine/OS/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "ThirdParty/DearImgui/imgui_impl_win32.h"


#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

//-----------------------------------------------------------------------------------------------
static TCHAR const* WND_CLASS_NAME = TEXT( "Simple Window Class" );


//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
// AKA WinProc
static LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	Window* window = (Window*) ::GetWindowLongPtr( windowHandle, GWLP_USERDATA );
	
	// TODO: Return if window is null and ensure input and event can never be null for a window
	// maybe use the config struct approach
	InputSystem* inputSystem = nullptr;
	EventSystem* eventSystem = nullptr;
	if ( window != nullptr )
	{
		inputSystem = window->GetInputSystem();
		eventSystem = window->GetEventSystem();
	}

	if ( ImGui_ImplWin32_WndProcHandler( windowHandle, wmMessageCode, wParam, lParam ) )
		return true;

	switch ( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			eventSystem->FireEvent( "Quit" );
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char)wParam;

			if( inputSystem->HandleKeyPressed( asKey ) )
			{
				return 0; 
			}

			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char)wParam;

			if ( inputSystem->HandleKeyReleased( asKey ) )
			{
				return 0;
			}

			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			// wParam tells us which buttons are down
			bool leftButtonDown = wParam & MK_LBUTTON;
			bool rightButtonDown = wParam & MK_RBUTTON;
			bool middleButtonDown = wParam & MK_MBUTTON;

			inputSystem->UpdateMouseButtonState( leftButtonDown, rightButtonDown, middleButtonDown );

			break;
		}

		case WM_MOUSEWHEEL:
		{
			short scrollFixedPoint = GET_WHEEL_DELTA_WPARAM( wParam ); // shift away low word part, leaving only the highword
			float scrollAmount = (float)scrollFixedPoint / WHEEL_DELTA; // convert to a numeric value
			inputSystem->AddMouseWheelScrollAmount( scrollAmount ); // let the input system know that the mouse wheel has moved

			break;
		}

		case WM_CHAR:
		{
			unsigned char asKey = (unsigned char)wParam;

			inputSystem->PushCharacter( asKey );
			return 0; 

			break;
		}

		case WM_PAINT:
		{
			// Here as formality, doesn't paint anything
			PAINTSTRUCT ps;
			BeginPaint( windowHandle, &ps );
			EndPaint( windowHandle, &ps );
		} return 1;  // handled it

		case WM_ACTIVATE:
		{
			if ( inputSystem == nullptr )
			{
				return 0;
			}

			short windowState = (short)LOWORD( wParam );

			if ( windowState == WA_ACTIVE 
				 || windowState == WA_CLICKACTIVE )
			{
				inputSystem->PopMouseOptions();
				return 1;
			}
			else if ( windowState == WA_INACTIVE )
			{
				inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//-----------------------------------------------------------------------------------------------
static void RegisterWindowClass()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>( WindowsMessageHandlingProcedure ); // Register our Windows message-handling function
	windowClassDescription.hInstance = ::GetModuleHandle( NULL );
	// SD2 BONUS: Icon code handled here
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = WND_CLASS_NAME;
	::RegisterClassEx( &windowClassDescription );
}


//-----------------------------------------------------------------------------------------------
static void UnregisterWindowClass()
{
	::UnregisterClass( WND_CLASS_NAME, ::GetModuleHandle( NULL ) );
}


//-----------------------------------------------------------------------------------------------
Window::Window()
{
	RegisterWindowClass();
	PopulateDesktopWindowData();
}


//-----------------------------------------------------------------------------------------------
void Window::PopulateDesktopWindowData()
{
	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	m_desktopWidth = (float)( desktopRect.right - desktopRect.left );
	m_desktopHeight = (float)( desktopRect.bottom - desktopRect.top );
	m_desktopAspect = m_desktopWidth / m_desktopHeight;
}


//-----------------------------------------------------------------------------------------------
Vec2 Window::GetClientWindowDimensions( float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode )
{
	float clientWidth = m_desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = m_desktopHeight * maxClientFractionOfDesktop;

	if ( windowMode == eWindowMode::WINDOWED )
	{
		if ( clientAspect > m_desktopAspect )
		{
			// Client window has a wider aspect than desktop; shrink client height to match its width
			clientHeight = clientWidth / clientAspect;
		}
		else
		{
			// Client window has a taller aspect than desktop; shrink client width to match its height
			clientWidth = clientHeight * clientAspect;
		}
	}

	return Vec2( clientWidth, clientHeight );
}


//-----------------------------------------------------------------------------------------------
Window::~Window()
{
	Close();
	UnregisterWindowClass();
}


//-----------------------------------------------------------------------------------------------
void Window::SetEventSystem( EventSystem* eventSystem )
{
	m_eventSystem = eventSystem;
}


//-----------------------------------------------------------------------------------------------
void Window::SetInputSystem( InputSystem* inputSystem )
{
	m_inputSystem = inputSystem;
}



//-----------------------------------------------------------------------------------------------
void GetWindowStyleFlagsForWindowMode( eWindowMode windowMode, DWORD& windowStyleFlags, DWORD& windowStyleExFlags )
{
	switch ( windowMode )
	{
		case eWindowMode::WINDOWED:
			windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
			windowStyleExFlags = WS_EX_APPWINDOW;
			break;

		case eWindowMode::BORDERLESS:
			windowStyleFlags = WS_POPUP;
			windowStyleExFlags = WS_EX_CLIENTEDGE | WS_EX_APPWINDOW;
			break;
	}
}


//-----------------------------------------------------------------------------------------------
void Window::GetAdjustedWindowEdges( float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode, 
									 long& windowLeft, long& windowRight, long& windowTop, long& windowBottom )
{
	if ( windowMode == eWindowMode::BORDERLESS )
	{
		maxClientFractionOfDesktop = 1.f;
	}

	Vec2 clientDimensions = GetClientWindowDimensions( clientAspect, maxClientFractionOfDesktop, windowMode );

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * ( m_desktopWidth - clientDimensions.x );
	float clientMarginY = 0.5f * ( m_desktopHeight - clientDimensions.y );
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientDimensions.x;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientDimensions.y;

	DWORD windowStyleFlags = 0;
	DWORD windowStyleExFlags = 0;
	GetWindowStyleFlagsForWindowMode( windowMode, windowStyleFlags, windowStyleExFlags );

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	windowLeft = windowRect.left;
	windowRight = windowRect.right;
	windowTop = windowRect.top;
	windowBottom = windowRect.bottom;
}


//-----------------------------------------------------------------------------------------------
void Window::UpdateClientWindowDimensions( float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode )
{
	Vec2 clientDimensions = GetClientWindowDimensions( clientAspect, maxClientFractionOfDesktop, windowMode );
	m_clientWidth = (unsigned int)clientDimensions.x;
	m_clientHeight = (unsigned int)clientDimensions.y;
}


//-----------------------------------------------------------------------------------------------
bool Window::Open( const std::string& title, float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode )
{
	DWORD windowStyleFlags = 0;
	DWORD windowStyleExFlags = 0;
	GetWindowStyleFlagsForWindowMode( windowMode, windowStyleFlags, windowStyleExFlags );

	long windowLeft = 0;
	long windowRight = 0;
	long windowTop = 0;
	long windowBottom = 0;
	GetAdjustedWindowEdges( clientAspect, maxClientFractionOfDesktop, windowMode, windowLeft, windowRight, windowTop, windowBottom );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, title.c_str(), -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		WND_CLASS_NAME,
		windowTitle,
		windowStyleFlags,
		windowLeft,
		windowTop,
		windowRight - windowLeft,
		windowBottom - windowTop,
		NULL,
		NULL,
		(HINSTANCE) ::GetModuleHandle(NULL),
		NULL );


	if ( hwnd == nullptr )
	{
		return false;
	}

	::SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)this );

	ShowWindow( hwnd, SW_SHOW );
	SetForegroundWindow( hwnd );
	SetFocus( hwnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	m_hwnd = (void*)hwnd;
	m_curWindowMode = windowMode;
	UpdateClientWindowDimensions( clientAspect, maxClientFractionOfDesktop, windowMode );

	return true;
}


//-----------------------------------------------------------------------------------------------
void Window::Close()
{
	HWND hwnd = (HWND)m_hwnd;
	if ( hwnd == nullptr )
	{
		return;
	}

	::DestroyWindow( hwnd );
	m_hwnd = nullptr;
}


void Window::BeginFrame()
{
	MSG queuedMessage;
	for ( ;; )
	{
		const BOOL wasMessagePresent = ::PeekMessage( &queuedMessage, (HWND)m_hwnd, 0, 0, PM_REMOVE );
		if ( !wasMessagePresent )
		{
			break;
		}

		::TranslateMessage( &queuedMessage );
		::DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}


//-----------------------------------------------------------------------------------------------
void Window::EndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void Window::ToggleWindowMode()
{	
	switch ( m_curWindowMode )
	{
		case eWindowMode::WINDOWED:	m_curWindowMode = eWindowMode::BORDERLESS; break;
		case eWindowMode::BORDERLESS: m_curWindowMode = eWindowMode::WINDOWED; break;
	}

	DWORD windowStyleFlags = 0;
	DWORD windowStyleExFlags = 0;
	GetWindowStyleFlagsForWindowMode( m_curWindowMode, windowStyleFlags, windowStyleExFlags );

	::SetWindowLongPtr( (HWND)m_hwnd, GWL_STYLE, (LONG_PTR)windowStyleFlags );
	::SetWindowLongPtr( (HWND)m_hwnd, GWL_EXSTYLE, (LONG_PTR)windowStyleExFlags );

	float windowAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 16.f / 9.f );
	float windowHeightRatio = g_gameConfigBlackboard.GetValue( "windowHeightRatio", .9f );
	
	long windowLeft = 0;
	long windowRight = 0;
	long windowTop = 0;
	long windowBottom = 0;
	GetAdjustedWindowEdges( windowAspect, windowHeightRatio, m_curWindowMode, windowLeft, windowRight, windowTop, windowBottom );

	::SetWindowPos( (HWND)m_hwnd, 0,
				  windowLeft,
				  windowTop,
				  windowRight - windowLeft,
				  windowBottom - windowTop,
				  SWP_FRAMECHANGED | SWP_SHOWWINDOW );
	
	UpdateClientWindowDimensions( windowAspect, windowHeightRatio, m_curWindowMode );
}

