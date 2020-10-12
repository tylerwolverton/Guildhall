#include "Game/AuthoritativeServer.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Startup( eAppMode appMode )
{
	switch ( appMode )
	{
		case eAppMode::SINGLE_PLAYER:
		{
			g_game = new SinglePlayerGame();
		}
		break;

		case eAppMode::MULTIPLAYER_SERVER:
		case eAppMode::HEADLESS_SERVER:
		{
			g_game = new MultiplayerGame();
		}
		break;

		case eAppMode::MULTIPLAYER_CLIENT:
		{
			ERROR_AND_DIE( "Cannot start an Authoritative server as a MultiplayerClient" );
		}
		break;
	}

	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Shutdown()
{
	g_game->Shutdown();

	PTR_SAFE_DELETE( g_game );
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Update()
{
	g_game->Update();
}
