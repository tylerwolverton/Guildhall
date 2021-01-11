#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Portal;
class World;
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
	Map( const MapData& mapData, World* world );
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

	void			UnloadAllEntityScripts();
	void			ReloadAllEntityScripts();

	Vec2 GetPlayerStartPos() const										{ return m_playerStartPos; }
	std::string GetName() const											{ return m_name; }

	void TakeOwnershipOfEntity( Entity* entityToAdd );
	void RemoveEntityFromMap( Entity* entityToRemove );
	void AddEntityToMap( Entity* entityToAdd );
	void AddInventoryItemsToMap( Entity* entity );
	void RemoveInventoryItemsFromMap( Entity* entity );

	void AddItemToTargetInventory( Entity* item, Entity* targetEntity );

	void WarpEntityInMap( Entity* entity, Portal* portal );

	Entity* GetEntityByName( const std::string& name );
	Entity* GetEntityById( EntityId id );

	Entity* GetEntityAtPosition( const Vec2& position );

private:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );
	
	void AddToEntityList( Entity* entity );

	void DeleteDeadEntities();

	virtual RaycastResult Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const = 0;

protected:
	std::string					m_name;
	World*						m_world = nullptr;

	// Multiplayer TODO: Make this into an array
	Vec2						m_playerStartPos = Vec2::ZERO;
	float						m_playerStartYaw = 0.f;

	Entity*						m_player = nullptr;
	std::vector<Entity*>		m_entities;
};