#include "Game/PlayerClient.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void PlayerClient::Startup()
{

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
