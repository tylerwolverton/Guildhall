#pragma once
#include "Engine/Core/EngineCommon.hpp"
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
	Server( EventArgs* args );
	virtual ~Server() = default;

	virtual void Startup( eAppMode appMode ) = 0;
	virtual void Shutdown();

	virtual void BeginFrame();
	virtual void Update() = 0;

	virtual void ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests ) = 0;

	// Change this to have the server own this client and the app doesn't access it?
	void SetPlayerClient( PlayerClient* playerClient )							{ m_playerClient = playerClient; }

protected:
	virtual void StartGame( eAppMode appMode ) = 0;
	virtual void ProcessNetworkMessages() = 0;

protected:
	PlayerClient* m_playerClient = nullptr;
	std::vector<RemoteClient*> m_remoteClients;

	std::string m_ipAddress;
	int m_tcpPort = -1;
};
