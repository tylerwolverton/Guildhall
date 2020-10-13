#pragma once
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
class KeyButtonState;


//-----------------------------------------------------------------------------------------------
class Server
{
public:
	Server() = default;
	virtual ~Server() = default;

	virtual void Startup( eAppMode appMode ) = 0;
	virtual void Shutdown();

	virtual void BeginFrame();
	virtual void Update() = 0;

	virtual void ReceiveInput( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos ) = 0;
};
