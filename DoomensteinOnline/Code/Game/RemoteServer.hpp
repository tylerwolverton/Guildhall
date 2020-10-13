#pragma once
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
class RemoteServer : public Server
{
public:
	RemoteServer() = default;
	~RemoteServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;
};
