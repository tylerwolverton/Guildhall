#pragma once
#include "Game/Entity.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
class ActorSpawner : public Entity
{
public:
	ActorSpawner( const EntityDefinition& entityDef, Map* curMap );
	virtual ~ActorSpawner();

	virtual void Update( float deltaSeconds );

private:

private:
	Timer m_cooldownTimer;
	int m_numWavesSpawned = 0;
};
