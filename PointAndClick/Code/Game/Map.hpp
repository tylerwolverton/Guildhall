#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Player;
class Actor;
class MapDefinition;


//-----------------------------------------------------------------------------------------------
enum class CardinalDirections
{
	CENTER,
	NORTH,
	EAST,
	SOUTH,
	WEST,
	NORTHWEST,
	NORTHEAST,
	SOUTHEAST,
	SOUTHWEST
};


//-----------------------------------------------------------------------------------------------
class Map
{

public:
	Map( std::string name, MapDefinition* mapDef );
	~Map();

	void Update( float deltaSeconds );
	void UpdateCameras();
	void Render() const;
	void DebugRender() const;

private:
	Actor*				SpawnNewActor( const Vec2& position, std::string actorName );
	void				SpawnPlayer();

	void				UpdateEntities( float deltaSeconds );
	void				UpdateMouseDebugInspection();

	void				RenderEntities() const;
	void				DebugRenderEntities() const;
	void				CenterCameraOnPlayer() const;
	
private:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	int					m_width = 0;
	int					m_height = 0;

	EntityVector		m_entities;
	Entity*				m_player;
};