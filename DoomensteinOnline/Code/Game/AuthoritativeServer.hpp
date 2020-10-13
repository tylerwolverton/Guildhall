#pragma once
#include "Game/Server.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class AuthoritativeServer : public Server
{
public:
	AuthoritativeServer() = default;
	~AuthoritativeServer() = default;

	virtual void Startup( eAppMode appMode ) override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void ReceiveInput( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos ) override;

private:
	const KeyButtonState* m_lastKeyStates = nullptr;
	Vec2 m_lastMouseDeltaPos = Vec2::ZERO;
};
