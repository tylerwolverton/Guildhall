#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
const char* APP_NAME = "FPS";						// ...becomes ??? (Change this per project!)

//Window* g_window = nullptr;

//-----------------------------------------------------------------------------------------------
void PopulateGameConfig()
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( "Data/GameConfig.xml" );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		return;
	}

	XmlElement* root = doc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *root );
}



//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE, _In_ LPSTR commandLineString, _In_ int )
{
	UNUSED( commandLineString );
	
	PopulateGameConfig();							

	g_app = new App();
	//g_window = new Window();
	//g_window->Open( APP_NAME, CLIENT_ASPECT, 0.9f ); // feed these from game blackboard
	
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
