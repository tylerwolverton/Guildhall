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
class Portal;
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

	void Load( Entity* player );
	void Unload();

	Entity* GetEntityByName( const std::string& name );

private:
	void				SpawnEntities();

	void				UpdateEntities( float deltaSeconds );
	void				UpdateMouseHover();
	void				CheckForTriggers();
	void				UpdateMouseDebugInspection();

	void				RenderEntities() const;
	void				DebugRenderEntities() const;
	void				CenterCameraOnPlayer() const;

	// Hacks
	void TiePortalToDoor();

	// Event handlers
	void				OnVerbAction( EventArgs* args );
	void				OnPickupVerb( EventArgs* args );
	void				OnOpenVerb( EventArgs* args );
	void				OnCloseVerb( EventArgs* args );
	void				OnTalkToVerb( EventArgs* args );
	void				OnGiveToSourceVerb( EventArgs* args );
	void				OnGiveToDestinationVerb( EventArgs* args );

private:
	std::string			 m_name;
	MapDefinition*       m_mapDef;

	int					 m_width = 0;
	int					 m_height = 0;

	EntityVector		 m_entities;
	std::vector<Item*>	 m_items;
	std::vector<Portal*> m_portals;
	Entity*				 m_player;

	std::vector<TriggerRegion> m_triggerRegions;
};
