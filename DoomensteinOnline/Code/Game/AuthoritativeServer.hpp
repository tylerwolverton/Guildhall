#pragma once
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
class AuthoritativeServer : public Server
{
public:
	AuthoritativeServer() = default;
	~AuthoritativeServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;
};
