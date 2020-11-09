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

	virtual void SetClientId( int id ) override;
	virtual void SetPlayer( Entity* entity ) override;

private:
	void ProcessUDPMessages();
};
