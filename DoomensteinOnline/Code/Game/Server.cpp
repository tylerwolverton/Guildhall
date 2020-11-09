#include "Game/Server.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Server::Server( EventArgs* args )
{
	if ( args != nullptr )
	{
		m_ipAddress = args->GetValue( "ip", "" );
		m_tcpPort = args->GetValue( "port", 48000 );
	}

	m_tcpId = m_rng.RollRandomIntInRange( 0, INT_MAX );
	m_udpId = m_rng.RollRandomIntInRange( 0, INT_MAX );
}


//-----------------------------------------------------------------------------------------------
void Server::Shutdown()
{
	g_game->Shutdown();

	PTR_SAFE_DELETE( g_game );
}


//-----------------------------------------------------------------------------------------------
void Server::BeginFrame()
{
	ProcessNetworkMessages();
}
