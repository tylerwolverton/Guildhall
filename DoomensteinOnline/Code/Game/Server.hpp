#pragma once
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Server
{
public:
	Server() = default;
	virtual ~Server() = default;

	virtual void Startup( eAppMode appMode ) = 0;
	virtual void Shutdown();

	virtual void Update() = 0;
};
