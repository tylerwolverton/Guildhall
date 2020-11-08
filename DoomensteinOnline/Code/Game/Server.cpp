#include "Game/Server.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Server::Server( EventArgs* args )
{
	if ( args != nullptr )
	{
		m_ipAddress = args->GetValue( "ip", "" );
		m_tcpPort = args->GetValue( "port", 48000 );
	}
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
