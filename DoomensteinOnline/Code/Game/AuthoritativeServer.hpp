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
	AuthoritativeServer() = default;
	~AuthoritativeServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests ) override;

private:
	const std::vector<ClientRequest*> m_clientRequests;
};
