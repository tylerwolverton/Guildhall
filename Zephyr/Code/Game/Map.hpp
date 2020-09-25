#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class Projectile;
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
	virtual ~Map();

	virtual void Load( Entity* player );
	virtual void Unload();

	virtual void Update( float deltaSeconds );
	virtual void UpdateMesh() = 0;
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual Entity* SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( const EntityDefinition& entityDef );
	virtual Entity* SpawnNewEntityOfTypeAtPosition( const std::string& entityDefName, const Vec2& pos );

	Vec2 GetPlayerStartPos() const										{ return m_playerStartPos; }

	void RemoveOwnershipOfEntity( Entity* entityToRemove );
	void TakeOwnershipOfEntity( Entity* entityToAdd );

	void WarpEntityInMap( Entity* entity, Portal* portal );

private:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );
	
	void AddToEntityList( Entity* entity );
	void AddToProjectileList( Projectile* projectile );
	void AddToPortalList( Portal* portal );
	void RemoveFromProjectileList( Projectile* projectile );
	void RemoveFromPortalList( Portal* portal );

	void DeleteDeadEntities();

	virtual RaycastResult Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const = 0;

protected:
	std::string					m_name;

	// Multiplayer TODO: Make this into an array
	Vec2						m_playerStartPos = Vec2::ZERO;
	float						m_playerStartYaw = 0.f;

	Entity*						m_player = nullptr;
	std::vector<Entity*>		m_entities;
	std::vector<Projectile*>	m_projectiles;
	std::vector<Portal*>		m_portals;
};
