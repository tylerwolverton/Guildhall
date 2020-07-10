#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class Portal;
class MapDefinition;


//-----------------------------------------------------------------------------------------------
struct RaycastResult
{
	Vec3 startPos;
	Vec3 forwardNormal;
	float maxDist = 0.f;
	bool didImpact = false;
	Vec3 impactPos;
	Entity* impactEntity = nullptr;
	float impactFraction = 0.f;
	float impactDist = 0.f;
	Vec3 impactSurfaceNormal;
};


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
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual Entity* SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( const EntityDefinition& entityDef );

	void RemoveOwnershipOfEntity( Entity* entityToRemove );
	void TakeOwnershipOfEntity( Entity* entityToAdd );

	Entity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );

protected:
	void LoadEntitiesFromDefinition();
	void ResolveEntityVsEntityCollisions();
	void ResolveEntityVsEntityCollision( Entity& entity1, Entity& entity2 );
	void ResolveEntityVsPortalCollisions();

	void WarpToNewMap( Entity* entity, Portal* portal );

	virtual RaycastResult Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const = 0;

protected:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	std::vector<Entity*> m_entities;
	std::vector<Portal*> m_portals;
};
