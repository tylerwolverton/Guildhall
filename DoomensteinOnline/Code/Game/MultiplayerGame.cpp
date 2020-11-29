#include "Game/MultiplayerGame.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/World.hpp"


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

	Map* map = entity->GetMap();
	if ( map == nullptr )
	{
		return;
	}

	Entity* targetEntity = map->GetEntityFromRaycast( Vec3( entity->GetPosition(), entity->GetEyeHeight() ), forwardVector, shotRange );
	if ( targetEntity == nullptr
		 || targetEntity->IsDead() )
	{
		return;
	}

	targetEntity->TakeDamage( damage );
	if ( targetEntity->IsDead() )
	{
		++m_playerScores[m_playerIdsToPlayerNums[shooterId]];
	}
}


//-----------------------------------------------------------------------------------------------
void MultiplayerGame::AddPlayerScore( int playerNum, EntityId playerId )
{
	m_playerIdsToPlayerNums[playerId] = playerNum;
	m_playerScores.push_back( 0 );
}


