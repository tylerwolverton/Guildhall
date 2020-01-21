#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/Tile.hpp"
#include "Game/FloodFillResult.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class World;
class Player;
class Texture;
class SpriteSheet;
struct RaycastImpact;
struct TileWorm;
struct MapEntityDefinition;
struct MapDefinition;


//-----------------------------------------------------------------------------------------------
enum class SurroundingTiles
{
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
	Map( int id, World* world, const MapDefinition& mapDefinition );
	~Map();

	void			Update( float deltaSeconds );
	void			UpdateCameras();
	void			Render() const;
	void			RenderHUD() const;
	void			DebugRender() const;

	int				GetId()													{ return m_id; }
	Entity*			GetPlayer()												{ return (Entity*)m_player; }   // Return as entity for now since only generic entity information is required and 
																											// callers shouldn't need to include Player.hpp (if specifics are needed this will be updated)

	// Entity management
	void			AddEntityToVector( Entity* entity, EntityType vectorType );
	void			RemoveEntityFromVector( Entity* entityToRemove, EntityType vectorType );
	Entity*			SpawnNewEntity( EntityType type, EntityFaction faction, const Vec2& position, float orientationDegrees = 0.f );
	void			SpawnExplosion( const Vec2& position, float radius, float durationSeconds );

	// Map creation / destruction
	void			LoadMap();
	void			UnloadMap();

	RaycastImpact	Raycast( const Vec2& start, const Vec2& forwardDirection, float maxDistance, float stepSize = 0.01f );
	RaycastImpact	VisionRaycast( const Vec2& start, const Vec2& forwardDirection, float maxDistance, float stepSize = 0.01f );
	bool			HasLineOfSight( const Entity& source, const Entity& target );

private:
	// Map building
	void			PopulateTiles( const MapDefinition& mapDefinition );
	void			CreateInitialTiles( TileType defaultTileype );
	void			DrawBorder( TileType edgeTileType );
	void			DeployWorms( std::vector<TileWorm> tileWorms );
	IntVec2			GetNextWormPosition( const IntVec2& currentPosition );
	void			ClearStartSafeZone( TileType startTileType, TileType shieldTileType );
	void			ClearEndSafeZone( TileType exitTileType, TileType shieldTileType );
	void			AddExitPoint();
	void			BuildSurroundingTileVector();
	void			FillInUnreachableTiles( TileType fillType );

	FloodFillResult FloodFillMap( const Vec2& startPos );
	void			SetFloodFillDataForTile( Tile* tile, FloodFillResult& result );

	// Spawning
	void			PopulateEnemySpawnLocations( const MapEntityDefinition& mapEntityDefinition );
	Vec2			GetRandomValidSpawnPosition();
	bool			IsPointInSafeZone( const Vec2& position );
	void			AddSpawnPositionToVector( const Vec2& spawnPosition, EntityType type );
	void			SpawnEnemies();

	void			DeleteGarbageEntities();

	// Tile helpers
	int				GetTileIndexFromTileCoords( int xCoord, int yCoord );
	int				GetTileIndexFromTileCoords( const IntVec2& coords );
	int				GetTileIndexFromWorldCoords( const Vec2& coords );

	Tile*			GetTileFromWorldCoords( const Vec2& coords );
	const Vec2		GetWorldCoordsFromTile( const Tile& tile );

	// Update
	void			UpdateEntities( float deltaSeconds );
	bool			CheckForWin();

	// Rendering
	void			RenderTiles() const;
	void			RenderEntities() const;
	void			DebugRenderEntities() const;
	void			CenterCameraOnPlayer() const;
	
	// Physics
	void			ResolveCollisions();
	void			ResolveEntityCollisions();
	void			ResolveTileCollisions();
	void			ResolveEntityCollision( Entity& a, Entity& b );
	void			ResolveEntityCollisionsWithSurroundingTiles( Entity& entity );
	void			ResolveEntityCollisionWithTile( Entity& entity, Vec2 tilePosition );
	void			ApplyTileEffectsToEntity( Entity& entity );
	void			ReflectBulletOffDisc( Entity& bullet, const Vec2& center, float radius );
	void			ResolveBulletEnvironmentCollisions();
	bool			IsBulletInsideWall( const Entity& entity );
	bool			IsBulletOutsideLevel( const Entity& bullet );

	bool			DoEntitiesOverlap( const Entity& entity1, const Entity& entity2 );
	void			PushEntityOutOfEntity( Entity& entityToPush, const Entity& entityPushing );
	void			PushEntitiesOutOfEachOther( Entity& entity1, Entity& entity2 );

	bool			IsPointInSolid( const Vec2& point );
	bool			IsPointInVisionBlockingSolid( const Vec2& point );
	bool			IsTileSolid( const Tile& tile );
	bool			DoesTileBlockVision( const Tile& tile );

private:
	int					m_id = -1;
	World*				m_world = nullptr;
	int					m_width = 0;
	int					m_height = 0;

	Vec2				m_exitPosition = Vec2::ZERO;

	EntityVector		m_entityVectorsByType[NUM_ENTITY_TYPES];
	std::vector<Vec2>	m_entitySpawnPositionVectorsByType[NUM_ENTITY_TYPES];

	Player*				m_player = nullptr;

	// Visual
	Texture*			m_tileTexture = nullptr;
	std::vector<Tile>	m_tiles;
	std::vector<Vec2>	m_surroundingTilePositions;
	FloodFillResult     m_floodFillResult;
};
