#pragma once
#include "Game/GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct ClientRequest;
class PlayerClient;
class RemoteClient;


//-----------------------------------------------------------------------------------------------
class Server
{
public:
	Server() = default;
	virtual ~Server() = default;

	virtual void Startup( eAppMode appMode ) = 0;
	virtual void Shutdown();

	virtual void BeginFrame();
	virtual void Update() = 0;

	virtual void ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests ) = 0;

	// Change this to have the server own this client and the app doesn't access it?
	void SetPlayerClient( PlayerClient* playerClient )							{ m_playerClient = playerClient; }

protected:
	PlayerClient* m_playerClient = nullptr;
	std::vector<RemoteClient*> m_remoteClients;
};
