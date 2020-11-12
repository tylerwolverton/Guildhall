#pragma once
#include "Game/Server.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class Client;
struct ClientRequest;


//-----------------------------------------------------------------------------------------------
struct ConnectionInfo
{
public:
	int key = -1;
	std::string ipAddress;
	int bindPort;
	int listenPort;

public:
	ConnectionInfo( int keyIn, const std::string& ipAddressIn, int bindPortIn, int listenPortIn )
		: key( keyIn )
		, ipAddress( ipAddressIn )
		, bindPort( bindPortIn )
		, listenPort( listenPortIn )
	{}
};


//-----------------------------------------------------------------------------------------------
class AuthoritativeServer : public Server
{
public:
	AuthoritativeServer( EventArgs* args );
	~AuthoritativeServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void ReceiveClientRequests( const std::vector<const ClientRequest*> clientRequests ) override;

	virtual void RegisterNewClient( Client* client ) override;

protected:
	virtual void StartGame( eAppMode appMode ) override;
	virtual void ProcessNetworkMessages() override;
	void ProcessTCPMessages();
	void ProcessUDPMessages();

private:

private:
	const std::vector<ClientRequest*>	m_clientRequests;

	std::vector<ConnectionInfo> m_clientConnectionInfo;
};
