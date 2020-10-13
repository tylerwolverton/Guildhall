#include "Game/PlayerClient.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
void PlayerClient::Startup()
{
	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );
	// Probably do something to hook up to RemoteServer if in MULTIPLAYER_CLIENT mode
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::BeginFrame()
{
	const KeyButtonState* keyStates = g_inputSystem->GetKeyStates();
	Vec2 mouseDeltaPos = g_inputSystem->GetMouseDeltaPosition();

	g_server->ReceiveInput( keyStates, mouseDeltaPos );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Render() const
{
	g_game->Render();
}
