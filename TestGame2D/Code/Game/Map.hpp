#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Player;


//-----------------------------------------------------------------------------------------------
class Map
{
public:
	Map( int width, int height );
	~Map();

	void Update( float deltaSeconds );
	void UpdateCameras();
	void Render() const;
	void DebugRender() const;

private:
	void PopulateTiles();
	void CreateInitialTiles();
	void DrawStoneBorder();
	void RandomlyDistributeStone();
	void ClearSafeZones();

	Entity* SpawnNewEntity( EntityType type, EntityFaction faction, const Vec2& position );
	void	SpawnPlayer();
	void	AddEntityToVector( Entity* entity, EntityType vectorType );

	int	GetTileIndexFromTileCoords( int xCoord, int yCoord );

	void UpdateEntities( float deltaSeconds );

	void RenderTiles() const;
	void RenderEntities() const;
	void DebugRenderEntities() const;
	void CenterCameraOnPlayer() const;
	
	void ResolveCollisions();
	void ResolveEntityCollisions( Entity& a, Entity& b );
	void ResolveEntityCollisionsWithSurroundingTiles( Entity* entity );
	void ResolveEntityCollisionWithTile( Entity* entity, Vec2 tilePosition );

private:
	int m_width;
	int m_height;

	std::vector<Tile> m_tiles;
	EntityVector m_entityVectorsByType[NUM_ENTITY_TYPES];

	Player* m_player = nullptr;
};