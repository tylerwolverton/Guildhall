#pragma once
#include "Game/GameCommon.hpp"

#include <string>
#include <map>
#include <unordered_map>


//-----------------------------------------------------------------------------------------------
struct Vec2;
class Clock;
class Entity;
class EntityDefinition;
class Map;
struct MapData;


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( Clock* gameClock );
	~World();

	void Update();
	void Render() const;
	void DebugRender() const;

	void AddNewMap( const MapData& mapData );
	void ChangeMap( const std::string& mapName, Entity* player );
	Map* GetMapByName( const std::string& name );
	Map* GetCurrentMap();

	void WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	bool IsMapLoaded( const std::string& mapName );

	void Reset();

	void UnloadAllEntityScripts();
	void ReloadAllEntityScripts();

	void InitializeAllZephyrEntityVariables();
	void CallAllZephyrSpawnEvents( Entity* player );
	
	void AddEntityFromDefinition( const EntityDefinition& entityDef );

	Entity* GetEntityById( EntityId id );
	Entity* GetEntityByIdInCurMap( EntityId id );
	Entity* GetEntityByName( const std::string& name );
	Entity* GetEntityByNameInCurMap( const std::string& name );

	void	SaveEntityByName( Entity* entity );

private:
	Map* GetLoadedMapByName( const std::string& mapName );

	void ClearMaps();
	void ClearEntities();

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;

	std::vector<Entity*> m_worldEntities;
	std::unordered_map<std::string, Entity*> m_entitiesByName;
	std::unordered_map<EntityId, Entity*> m_entitiesById;
};
