#include "Game/Server.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
void Server::Shutdown()
{
	g_game->Shutdown();

	PTR_SAFE_DELETE( g_game );
}


//-----------------------------------------------------------------------------------------------
void Server::BeginFrame()
{

}
