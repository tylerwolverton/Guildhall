#pragma once
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Player;
struct MapDefinition;


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World();
	~World();

	void Update( float deltaSeconds );
	void Render() const;
	void RenderHUD() const;
	void DebugRender() const;

	void GenerateMaps();
	void ChangeMap( int mapId );
	void ChangeMap();

	Player* GetPlayer() const;

private:
	void BuildNewMap( const MapDefinition& mapDefinition );

private:
	std::vector<Map*>	m_maps;
	Map*				m_curMap = nullptr;
};