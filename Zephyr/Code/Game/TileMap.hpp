#pragma once
#include "Game/Map.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
struct MapData;
class GPUMesh;
class Material;
class TileDefinition;


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
	TileMap( const MapData& mapData, World* world );
	virtual ~TileMap();

	virtual void Load( Entity* player ) override;
	virtual void Unload() override;

	virtual void Update( float deltaSeconds ) override;
	virtual void UpdateMesh() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	virtual Vec2 GetDimensions() const override;

	virtual RaycastResult	Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const override;
	RaycastResult			RaycastAgainstWalls( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const;
	//RaycastResult			RaycastAgainstEntities( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const;
	//RaycastResult			RaycastAgainstEntitiesFast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const;

private:
	void						PopulateTiles( const std::vector<TileDefinition*>& tileDefs );
	void						CreateInitialTiles( const std::vector<TileDefinition*>& tileDefs );

	void UpdateCameras();
	void CenterCameraOnPlayer() const;

	// Tile helpers
	bool						IsAdjacentTileSolid( const Tile& tile, eCardinalDirection direction ) const;
	bool						IsTileSolid( int xCoord, int yCoord ) const;
	int							GetTileIndexFromTileCoords( int xCoord, int yCoord ) const;
	int							GetTileIndexFromTileCoords( const IntVec2& coords ) const;
	int							GetTileIndexFromWorldCoords( const Vec2& coords ) const;

	const Tile*					GetTileFromTileCoords( const IntVec2& tileCoords ) const;
	const Tile*					GetTileFromTileCoords( int xCoord, int yCoord ) const;
	const Tile*					GetTileFromWorldCoords( const Vec2& worldCoords ) const;
	const Vec2					GetWorldCoordsFromTile( const Tile& tile ) const;

	std::vector<const Tile*>	GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile ) const;
	
	void						BuildCardinalDirectionsArray();
	
	void						CreateTileRigidbodies();
	void						DestroyTileRigidbodies();

private:
	Transform				m_raytraceTransform;

	std::vector<Tile>		m_tiles;
	IntVec2					m_dimensions;

	Vec2					m_cardinalDirectionOffsets[9];

	std::vector<Vertex_PCU> m_mesh;

	std::vector<Rigidbody2D*> m_tileRigidbodies;
};
