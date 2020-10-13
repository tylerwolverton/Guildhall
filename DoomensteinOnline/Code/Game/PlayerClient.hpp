#pragma once
#include "Game/Client.hpp"


//-----------------------------------------------------------------------------------------------
class PlayerClient : public Client
{
public:
	PlayerClient() = default;
	~PlayerClient() = default;

	virtual void Startup();
	virtual void Shutdown();

	virtual void Render() const override;
};
