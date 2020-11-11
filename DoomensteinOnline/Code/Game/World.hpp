#pragma once
#include "Game/GameCommon.hpp"

#include <string>
#include <map>
#include <vector>


//-----------------------------------------------------------------------------------------------
enum eEntityType : int;
struct Vec2;
class Clock;
class Entity;
class Map;
struct MapData;


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( Clock* gameClock );
	~World();

	void Update();
	void UpdateMesh();
	void Render() const;
	void DebugRender() const;

	void AddNewMap( const MapData& mapData );
	void ChangeMap( const std::string& mapName );
	
	Entity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );

	void WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	bool IsMapLoaded( const std::string& mapName );

	Entity* CreateEntityInCurrentMap( eEntityType entityType, const Vec2& position, float yawOrientationDegrees );
	Entity* CreateEntityInCurrentMap( EntityId id, eEntityType entityType, const Vec2& position, float yawOrientationDegrees );
	std::vector<Entity*> GetEntitiesInCurrentMap();

	void AddEntity( Entity* entity );
	Entity* GetEntityById( EntityId entityId );

	void DeleteAllEntities();

private:
	Map* GetLoadedMapByName( const std::string& mapName );

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;
	std::map<EntityId, Entity*> m_entities;
};