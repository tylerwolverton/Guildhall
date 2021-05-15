#include "Game/ActorSpawner.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
ActorSpawner::ActorSpawner( const EntityDefinition& entityDef, Map* curMap )
	: Entity( entityDef, curMap )
{
	m_cooldownTimer.Start( entityDef.GetSpawnCooldown() );
}


//-----------------------------------------------------------------------------------------------
ActorSpawner::~ActorSpawner()
{
}


//-----------------------------------------------------------------------------------------------
void ActorSpawner::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	//if ( m_cooldownTimer.HasElapsed()
	//	 && m_numWavesSpawned < m_entityDef.GetMaxSpawnWaves() )
	//{
	//	for ( const SpawnTarget& spawnTarget : m_entityDef.GetSpawnTargets() )
	//	{
	//		Actor* newEntity = (Actor*)m_map->SpawnNewEntityOfType( spawnTarget.entityDefName );
	//		newEntity->SetInitialPosition( GetPosition() + g_game->m_rng->RollRandomDirection2D() );
	//		newEntity->SetMoveTargetPosition( newEntity->GetPosition() );
	//	}

	//	m_cooldownTimer.Start( m_entityDef.GetSpawnCooldown() );
	//	++m_numWavesSpawned;
	//}
}

