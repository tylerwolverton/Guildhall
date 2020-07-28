#pragma once
#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Clock;
class Map;
class Actor;
class Cursor;
class Entity;

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
	void ChangeMap( const std::string& mapName, Actor* player, Cursor* cursor );

	void ReloadMaps();
	void UnloadCurrentMap();

	Entity* GetPurpleTentacle();

private:
	Map* GetLoadedMapByName( const std::string& mapName );

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;
};
