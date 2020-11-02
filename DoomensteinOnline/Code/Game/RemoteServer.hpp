#pragma once
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
class RemoteServer : public Server
{
public:
	RemoteServer( EventArgs* args );
	~RemoteServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests ) override;

protected:
	virtual void StartGame( eAppMode appMode ) override;

private:
	void NegotiateUDPConnection();
};
