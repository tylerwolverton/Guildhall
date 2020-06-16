#pragma once
#include "Game/Map.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
class MapDefinition;
class GPUMesh;
class Material;

//-----------------------------------------------------------------------------------------------
enum class eCardinalDirection
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
class TileMap : public Map
{
public:
	TileMap( std::string name, MapDefinition* mapDef );
	virtual ~TileMap();

	virtual void Load() override;
	virtual void Unload() override;

	virtual void UpdateMeshes() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

private:
	void				PopulateTiles();
	void				CreateInitialTiles();
	void				SolidifySurroundingTiles();

	void				AddTileFace( const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topLeft, const Vec3& topRight, const Vec2& uvMins = Vec2::ZERO, const Vec2& uvMaxs = Vec2::ONE );

	void				SpawnPlayer();

	// Tile helpers
	bool				IsAdjacentTileSolid( const Tile& tile, eCardinalDirection direction );
	int					GetTileIndexFromTileCoords( int xCoord, int yCoord );
	int					GetTileIndexFromTileCoords( const IntVec2& coords );
	int					GetTileIndexFromWorldCoords( const Vec2& coords );

	Tile*				GetTileFromTileCoords( const IntVec2& tileCoords );
	Tile*				GetTileFromTileCoords( int xCoord, int yCoord );
	Tile*				GetTileFromWorldCoords( const Vec2& worldCoords );
	const Vec2			GetWorldCoordsFromTile( const Tile& tile );

	std::vector<Tile*>	GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile );
	
	void				RenderTiles() const;

	void				BuildCardinalDirectionsArray();
	void				CreateTestBoxes();
	void				RenderTestBoxes() const;

private:
	std::vector<Tile>	m_tiles;

	Vec2				m_cardinalDirectionOffsets[9];

	// For cube tests
	GPUMesh* m_cubeMesh = nullptr;
	Material* m_testMaterial = nullptr;

	std::vector<Transform> m_cubeMeshTransforms;

	std::vector<Vertex_PCU> m_mesh;
};
