#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Player;
class Actor;
class Item;
class MapDefinition;
class TriggerRegion;


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
	void				SpawnPlayer();

	void				UpdateEntities( float deltaSeconds );
	void				UpdateMouseHover();
	void				CheckForTriggers();
	void				UpdateMouseDebugInspection();

	void				RenderEntities() const;
	void				DebugRenderEntities() const;
	void				CenterCameraOnPlayer() const;
	
	void				PickUpItem( const Vec2& worldPosition );

	// Event handlers
	void				OnVerbAction( EventArgs* args );
	void				OnPickUpItem( EventArgs* args );

private:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	int					m_width = 0;
	int					m_height = 0;

	EntityVector		m_entities;
	std::vector<Item*>	m_items;
	Entity*				m_player;

	std::vector<TriggerRegion> m_triggerRegions;
};
