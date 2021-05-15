#pragma once
#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
struct Vec2;
class Clock;
class Entity;
class Map;
struct MapData;
enum class eBroadphaseCheckType;
enum class eNarrowphaseCheckType;


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( Clock* gameClock );
	~World();

	void Update();
	void Render() const;
	void DebugRender() const;

	void AddNewMap( const std::string& mapName );
	void ChangeMap( const std::string& mapName );

	bool IsMapLoaded( const std::string& mapName );

	void ResetCurrentMap();
	void AddEntitySet( int numEntities );
	void RemoveEntitySet( int numEntities );

	int GetObjectCount() const;
	int GetRaycastCount() const;
	double GetRaycastTimeMs() const;
	int GetNumRaycastImpacts() const;

	void CycleBroadphaseCheck();
	eBroadphaseCheckType GetBroadphaseCheckType() const;
	std::string GetBroadphaseCheckTypeStr() const;

	void CycleNarrowphaseCheck();
	eNarrowphaseCheckType GetNarrowphaseCheckType() const;
	std::string GetNarrowphaseCheckTypeStr() const;

	void SaveConvexSceneToFile( const std::string& fileName );
	void LoadConvexSceneFromFile( const std::string& fileName );

private:
	Map* GetLoadedMapByName( const std::string& mapName );

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;
};
