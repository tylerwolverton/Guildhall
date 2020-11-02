#pragma once
#include "Game/Server.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct ClientRequest;


//-----------------------------------------------------------------------------------------------
class AuthoritativeServer : public Server
{
public:
	AuthoritativeServer( EventArgs* args );
	~AuthoritativeServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests ) override;

protected:
	virtual void StartGame( eAppMode appMode ) override;

private:
	void StartTCPServer();

private:
	const std::vector<ClientRequest*> m_clientRequests;
};
