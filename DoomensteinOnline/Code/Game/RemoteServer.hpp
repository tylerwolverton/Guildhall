#pragma once
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
class TCPSocket;


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

	virtual void ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests ) override;

	virtual void RegisterNewClient( Client* client ) override;

protected:
	virtual void StartGame( eAppMode appMode ) override;
	virtual void ProcessNetworkMessages() override;
	void ProcessTCPMessages();
	void ProcessUDPMessages();

private:
	void NegotiateUDPConnection();

private:
	Client* m_playerClient = nullptr;

	TCPSocket* m_tcpClientSocket = nullptr;
};
