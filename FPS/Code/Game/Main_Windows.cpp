#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>

#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE, _In_ LPSTR commandLineString, _In_ int )
{
	UNUSED( commandLineString );
							
	g_app = new App();	
	g_app->Startup();
	
	// Program main loop; keep running frames until it's time to quit
	while( !g_app->IsQuitting() )			
	{
		//Sleep( 16 );							// Do nothing for 16 ms
		g_app->RunFrame();					
	}

	g_app->Shutdown();
	delete g_app;
	g_app = nullptr;
	
	return 0;
}
