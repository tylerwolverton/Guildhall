#pragma once
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
class PlayerClient;


//-----------------------------------------------------------------------------------------------
class RemoteServer : public Server
{
public:
	RemoteServer( EventArgs* args );
	~RemoteServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void BeginFrame() override;
	virtual void Update() override;

	virtual void ReceiveClientRequests( const std::vector<const ClientRequest*> clientRequests ) override;

	virtual void RegisterNewClient( Client* client ) override;

protected:
	virtual void StartGame( eAppMode appMode ) override;
	virtual void ProcessNetworkMessages() override;
	void ProcessTCPMessages();
	void ProcessUDPMessages();

private:
	void RequestUDPConnection();

private:
	PlayerClient* m_playerClient = nullptr;

	int m_remoteClientId = -1;

	int m_udpSendToPort = 4800;
};
