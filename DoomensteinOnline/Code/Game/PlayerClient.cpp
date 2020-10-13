#include "Game/PlayerClient.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void PlayerClient::Startup()
{
	// Probably do something to hook up to RemoteServer if in MULTIPLAYER_CLIENT mode
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Render() const
{
	g_game->Render();
}
