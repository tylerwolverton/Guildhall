#include "Engine/OS/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"


#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places


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
	
	InputSystem* inputSystem = nullptr;
	EventSystem* eventSystem = nullptr;
	if ( window != nullptr )
	{
		inputSystem = window->GetInputSystem();
		eventSystem = window->GetEventSystem();
	}

	switch ( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			EventArgs args;
			eventSystem->FireEvent( "QuitGame", &args );
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char)wParam;

			if( inputSystem->HandleKeyPressed( asKey ) )
			{
				return 0; // "Consumes" this message (tells Windows "okay, we handled it")
			}

			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char)wParam;

			if ( inputSystem->HandleKeyReleased( asKey ) )
			{
				return 0; // "Consumes" this message (tells Windows "okay, we handled it")
			}

			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint( windowHandle, &ps );
			EndPaint( windowHandle, &ps );
		} return 1;  // handled it
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
bool Window::Open( const std::string& title, float clientAspect, float maxClientFractionOfDesktop, WindowMode windowMode )
{
	DWORD windowStyleFlags = 0;
	DWORD windowStyleExFlags = 0;

	switch ( windowMode )
	{
		case WindowMode::WINDOWED:
			windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
			windowStyleExFlags = WS_EX_APPWINDOW;
			break;

		case WindowMode::BORDERLESS:
			windowStyleFlags = WS_POPUP;
			windowStyleExFlags = WS_EX_CLIENTEDGE | WS_EX_APPWINDOW;
			maxClientFractionOfDesktop = 1.f;
			break;
	}
	
	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)( desktopRect.right - desktopRect.left );
	float desktopHeight = (float)( desktopRect.bottom - desktopRect.top );
	float desktopAspect = desktopWidth / desktopHeight;
	
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;

	if ( windowMode == WindowMode::WINDOWED )
	{
		if ( clientAspect > desktopAspect )
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

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * ( desktopWidth - clientWidth );
	float clientMarginY = 0.5f * ( desktopHeight - clientHeight );
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	m_clientWidth = (unsigned int)clientWidth;
	m_clientHeight = (unsigned int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, title.c_str(), -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		WND_CLASS_NAME,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
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
