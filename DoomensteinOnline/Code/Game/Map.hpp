#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class Portal;
class World;
struct MapData;
struct MapEntityDefinition;


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
	Map( const MapData& mapData, World* world );
	virtual ~Map();

	virtual void Load() = 0;
	virtual void Unload() = 0;

	virtual void Update( float deltaSeconds );
	virtual void UpdateMeshes() = 0;
	virtual void UpdateEntityAnimations();
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual Entity* SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( EntityId id, const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( const EntityDefinition& entityDef );
	virtual Entity* SpawnNewEntityOfType( EntityId id, const EntityDefinition& entityDef );

	void RemoveOwnershipOfEntity( Entity* entityToRemove );
	void TakeOwnershipOfEntity( Entity* entityToAdd );

	Entity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );
	std::vector<Entity*> GetAllEntities();
	std::vector<Entity*> GetAllLivingEntities();
	void DeleteAllEntities();

	virtual RaycastResult Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const = 0;
	virtual Entity* GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const = 0;

protected:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );
	void ResolveEntityVsEntityCollisions();
	void ResolveEntityVsEntityCollision( Entity& entity1, Entity& entity2 );
	void ResolveEntityVsPortalCollisions();
	void CleanupDeadEntities();

	void WarpEntityInMap( Entity* entity, Portal* portal );

protected:
	World*				 m_world;
	std::string			 m_name;

	// Multiplayer TODO: Make this into an array
	Vec2				 m_playerStartPos = Vec2::ZERO;
	float				 m_playerStartYaw = 0.f;

	std::vector<Entity*> m_entities;
	std::vector<Entity*> m_livingEntities;
	std::vector<Portal*> m_portals;
};
