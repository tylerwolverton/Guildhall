#pragma once
#include "Game/Client.hpp"
#include "Game/AuthoritativeServer.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
enum eInitializationState
{
	NOT_SENT,
	SENT,
	ACKED
};


//-----------------------------------------------------------------------------------------------
struct ClientRequest;


//-----------------------------------------------------------------------------------------------
class RemoteClient : public Client
{
public:
	RemoteClient( ConnectionInfo connectionInfo );
	~RemoteClient() = default;

	virtual void Startup();
	virtual void Shutdown();

	virtual void Update() override;

	virtual void SendMessageToDistantClient( ClientRequest* message ) override;

	virtual void SetClientId( int id ) override;
	virtual void SetPlayer( Entity* entity ) override;

private:
	void ProcessUDPMessages();

private:
	ConnectionInfo m_connectionInfo;
	bool m_hasSentInitialState = false;

	EntityId m_playerId = -1;
	eInitializationState m_remoteServerInitState = eInitializationState::NOT_SENT;
	eInitializationState m_remoteServerPlayerIdInitState = eInitializationState::NOT_SENT;

	double m_lastUpdateTime = 1.f;
};
