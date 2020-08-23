#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class Portal;
struct MapData;
struct MapEntityDefinition;


//-----------------------------------------------------------------------------------------------
struct RaycastResult
{
	Vec2 startPos;
	Vec2 forwardNormal;
	float maxDist = 0.f;
	bool didImpact = false;
	Vec2 impactPos;
	Entity* impactEntity = nullptr;
	float impactFraction = 0.f;
	float impactDist = 0.f;
	Vec2 impactSurfaceNormal;
};


//-----------------------------------------------------------------------------------------------
class Map
{
public:
	Map( const MapData& mapData );
	~Map();

	virtual void Load( Entity* player );
	virtual void Unload();

	virtual void Update( float deltaSeconds );
	virtual void UpdateMesh() = 0;
	void UpdateCameras();
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual Entity* SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( const EntityDefinition& entityDef );

	void RemoveOwnershipOfEntity( Entity* entityToRemove );
	void TakeOwnershipOfEntity( Entity* entityToAdd );

private:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );
	void ResolveEntityVsEntityCollisions();
	void ResolveEntityVsEntityCollision( Entity& entity1, Entity& entity2 );
	void ResolveEntityVsPortalCollisions();

	void WarpEntityInMap( Entity* entity, Portal* portal );

	virtual RaycastResult Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const = 0;

protected:
	std::string			 m_name;

	// Multiplayer TODO: Make this into an array
	Vec2				 m_playerStartPos = Vec2::ZERO;
	float				 m_playerStartYaw = 0.f;

	Entity*				 m_player = nullptr;
	std::vector<Entity*> m_entities;
	std::vector<Portal*> m_portals;
};
