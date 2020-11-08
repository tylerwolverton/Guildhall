#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct ClientRequest;
class Client;


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
	
	virtual void RegisterNewClient( Client* client ) = 0;

protected:
	virtual void StartGame( eAppMode appMode ) = 0;
	virtual void ProcessNetworkMessages() = 0;

protected:
	std::vector<Client*>	m_clients;

	std::string				m_ipAddress;
	int						m_tcpPort = -1;
};
