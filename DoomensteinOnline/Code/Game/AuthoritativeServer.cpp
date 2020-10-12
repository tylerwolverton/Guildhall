#include "Game/AuthoritativeServer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Startup()
{
	g_game = new Game();
	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Shutdown()
{
	g_game->Shutdown();

	PTR_SAFE_DELETE( g_game );
}

