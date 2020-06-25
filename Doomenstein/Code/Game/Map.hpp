#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class MapDefinition;


//-----------------------------------------------------------------------------------------------
class Map
{
public:
	Map( std::string name, MapDefinition* mapDef );
	virtual ~Map();

	virtual void Load() = 0;
	virtual void Unload() = 0;

	virtual void Update( float deltaSeconds );
	virtual void UpdateMeshes() = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const;

	virtual Entity* SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( const EntityDefinition& entityDef );

	Entity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );

protected:
	void LoadEntitiesFromDefinition();
	void ResolveEntityVsEntityCollisions();
	void ResolveEntityVsEntityCollision( Entity& entity1, Entity& entity2 );

protected:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	std::vector<Entity*> m_entities;
};
