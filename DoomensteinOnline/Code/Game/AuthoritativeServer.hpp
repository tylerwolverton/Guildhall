#pragma once
#include "Game/Server.hpp"


//-----------------------------------------------------------------------------------------------
class AuthoritativeServer : public Server
{
public:
	AuthoritativeServer() = default;
	~AuthoritativeServer() = default;

	virtual void Startup() override;
	virtual void Shutdown() override;
};
