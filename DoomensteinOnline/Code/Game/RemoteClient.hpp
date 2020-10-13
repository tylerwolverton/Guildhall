#pragma once
#include "Game/Client.hpp"


//-----------------------------------------------------------------------------------------------
class RemoteClient : public Client
{
public:
	RemoteClient() = default;
	~RemoteClient() = default;

	virtual void Startup();
	virtual void Shutdown();
};
