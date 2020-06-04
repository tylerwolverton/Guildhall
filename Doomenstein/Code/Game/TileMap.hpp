#pragma once
#include "Game/Map.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
class MapDefinition;
class GPUMesh;
class Material;

//-----------------------------------------------------------------------------------------------
enum class eCardinalDirections
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

	virtual void UpdateMeshes() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

private:
	void				PopulateTiles();
	void				CreateInitialTiles();

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
