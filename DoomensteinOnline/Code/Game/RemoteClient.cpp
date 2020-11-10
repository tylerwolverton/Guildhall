#include "Game/RemoteClient.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Networking/NetworkingSystem.hpp"
#include "Game/Server.hpp"
#include "Game/GameEvents.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void RemoteClient::Startup()
{
	// Some config to talk to server
	/*RemoteClientRegistrationRequest req( m_clientId );

	g_networkingSystem->SendUDPMessage( 4820, &req, sizeof( req ) );*/
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void RemoteClient::Update()
{
	ProcessUDPMessages();
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetClientId( int id )
{
	m_clientId = id;
	// Send a message to RemoteServer to set player client's id
	RemoteClientRegistrationRequest req( m_clientId );

	g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetPlayer( Entity* entity )
{
	UNUSED( entity );

	g_devConsole->PrintError( "Cannot set player on a non player client" );
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::ProcessUDPMessages()
{
	std::vector<const ClientRequest*> gameRequests;

	std::vector<UDPData>& newMessages = g_networkingSystem->ReceiveUDPMessages();

	for ( UDPData& data : newMessages )
	{
		if ( data.GetData() == nullptr )
		{
			continue;
		}

		const ClientRequest* req = reinterpret_cast<const ClientRequest*>( data.GetPayload() );
		gameRequests.push_back( req );

		data.Process();
	}

	g_server->ReceiveClientRequests( gameRequests );
}

