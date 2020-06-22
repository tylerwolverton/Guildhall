#pragma once

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
struct Vec2;
class Clock;
class Entity;
class Map;
class MapDefinition;

//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( Clock* gameClock );
	~World();

	void Update();
	void Render() const;
	void DebugRender() const;

	void LoadMap( const std::string& mapName );
	void LoadMap( const std::string& mapName, MapDefinition* mapDef );
	void ChangeMap( const std::string& mapName );

	Entity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );

private:
	Map* GetLoadedMapByName( const std::string& mapName );

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;
};