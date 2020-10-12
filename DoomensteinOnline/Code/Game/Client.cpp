#include "Game/Client.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerClient.hpp"
#include "Game/RemoteClient.hpp"


//-----------------------------------------------------------------------------------------------
void Client::Startup()
{
	// Break these into separate classes?
	m_playerClient = new PlayerClient();
	m_playerClient->Startup();

	m_remoteClient = new RemoteClient();
	m_remoteClient->Startup();
}


//-----------------------------------------------------------------------------------------------
void Client::Shutdown()
{
	m_playerClient->Shutdown();
	m_remoteClient->Shutdown();

	PTR_SAFE_DELETE( m_playerClient );
	PTR_SAFE_DELETE( m_remoteClient );
}


//-----------------------------------------------------------------------------------------------
void Client::Render() const
{
	m_playerClient->Render();
}
