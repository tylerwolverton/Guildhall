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
	friend class MapGenStep_Mutate;
	friend class MapGenStep_Worm;
	friend class MapGenStep_FromImage;
	friend class MapGenStep_CellularAutomata;
	friend class MapGenStep_RoomsAndPaths;

public:
	Map( std::string name, MapDefinition* mapDef );
	~Map();

	void Update( float deltaSeconds );
	void UpdateCameras();
	void Render() const;
	void DebugRender() const;

private:
	void				PopulateTiles();
	void				CreateInitialTiles();
	void				SetEdgeTiles();

	Actor*				SpawnNewActor( const Vec2& position, std::string actorName );
	void				SpawnPlayer();

	// Tile helpers
	int					GetTileIndexFromTileCoords( int xCoord, int yCoord );
	int					GetTileIndexFromTileCoords( const IntVec2& coords );
	int					GetTileIndexFromWorldCoords( const Vec2& coords );

	Tile*				GetTileFromTileCoords( const IntVec2& tileCoords );
	Tile*				GetTileFromTileCoords( int xCoord, int yCoord );
	Tile*				GetTileFromWorldCoords( const Vec2& worldCoords );
	const Vec2			GetWorldCoordsFromTile( const Tile& tile );

	std::vector<Tile*>	GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile );

	void				UpdateEntities( float deltaSeconds );
	void				UpdateMouseDebugInspection();

	void				RenderTiles() const;
	void				RenderEntities() const;
	void				DebugRenderEntities() const;
	void				CenterCameraOnPlayer() const;
	
	// Physics
	void				BuildCardinalDirectionsArray();
	void				ResolveCollisions();
	void				ResolveEntityCollisionsWithSurroundingTiles( Entity& entity );
	void				ResolveEntityCollisionWithTile( Entity& entity, Vec2 tilePosition );

	bool				CanEntityEnterTile( const Entity& entity, const Tile& tile );

private:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	int					m_width = 0;
	int					m_height = 0;

	std::vector<Tile>	m_tiles;
	EntityVector		m_entities;
	Entity*				m_player;

	// Physics
	Vec2				m_cardinalDirectionOffsets[9];
};