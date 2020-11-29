#include "Game/MultiplayerGame.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/World.hpp"
#include "Game/GameEvents.hpp"
#include "Game/Server.hpp"
#include "Game/PlayerClient.hpp"


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::Startup()
{
	Game::Startup();
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::Update()
{
	Game::Update();
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::Render() const
{
	Game::Render();
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::RestartGame()
{
	Game::RestartGame();
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::ShootEntity( EntityId shooterId, const Vec3& forwardVector, float shotRange, int damage )
{
	Entity* entity = m_world->GetEntityById( shooterId );

	if ( entity == nullptr )
	{
		return;
	}

	int playerNum = m_playerIdsToPlayerNums[shooterId];

	Map* map = entity->GetMap();
	if ( map == nullptr )
	{
		return;
	}

	//Entity* targetEntity = map->GetEntityFromRaycast( Vec3( entity->GetPosition(), entity->GetEyeHeight() ), forwardVector, shotRange );
	RaycastResult shotResult = map->Raycast( Vec3( entity->GetPosition(), entity->GetEyeHeight() ), forwardVector, shotRange );

	if ( shotResult.didImpact )
	{
		Vec3 start = shotResult.startPos;
		start.z -= .1f;
		g_playerClient->DrawShot( start, shotResult.impactPos, g_playerClient->GetColorForPlayer( playerNum ) );
	}
	else
	{
		Vec3 start( entity->GetPosition(), entity->GetEyeHeight() - .1f );
		g_playerClient->DrawShot( start, start + ( forwardVector * shotRange ), g_playerClient->GetColorForPlayer( playerNum ) );
	}

	ClientRequest* drawShotReq = new DrawShotRequest( -1, shotResult.startPos, shotResult.impactPos, g_playerClient->GetColorForPlayer( playerNum ) );
	g_server->SendMessageToAllDistantClients( drawShotReq );

	PTR_SAFE_DELETE( drawShotReq );

	Entity* targetEntity = shotResult.impactEntity;
	if ( targetEntity == nullptr
		 || targetEntity->IsDead() )
	{
		return; 
	}

	targetEntity->TakeDamage( damage );
	if ( targetEntity->IsDead() )
	{
		++m_playerScores[playerNum];

		ClientRequest* updateScoresReq = new UpdatePlayerScoreRequest( -1, playerNum, m_playerScores[playerNum] );
		g_server->SendMessageToAllDistantClients( updateScoresReq );

		PTR_SAFE_DELETE( updateScoresReq );
	}
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::AddPlayerScore( int playerNum, EntityId playerId )
{
	m_playerIdsToPlayerNums[playerId] = playerNum;
	m_playerScores.push_back( 0 );
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::UpdatePlayerScore( int playerNum, int newScore )
{
	if ( playerNum < 0 )
	{
		return;
	}

	// Expand player scores array to handle new players
	if ( playerNum >= (int)m_playerScores.size() )
	{
		int numNewPlayers = playerNum - ( (int)m_playerScores.size() - 1 );
		for ( int newPlayerIdx = 0; newPlayerIdx < numNewPlayers; ++newPlayerIdx )
		{
			m_playerScores.push_back( 0 );
		}
	}

	m_playerScores[playerNum] = newScore;
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::UpdatePlayerScoresForAllClients()
{
	for ( int playerIdx = 0; playerIdx < (int)m_playerScores.size(); ++playerIdx )
	{
		ClientRequest* updateScoresReq = new UpdatePlayerScoreRequest( -1, playerIdx, m_playerScores[playerIdx] );
		g_server->SendMessageToAllDistantClients( updateScoresReq );

		PTR_SAFE_DELETE( updateScoresReq );
	}
}


