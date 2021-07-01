#pragma once
#include "Game/Map.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
struct MapData;
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
	TileMap( const MapData& mapData, World* world );
	virtual ~TileMap();

	virtual void Load() override;
	virtual void Unload() override;

	virtual void Update( float deltaSeconds ) override;
	virtual void UpdateMeshes() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	virtual RaycastResult Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const override;
	RaycastResult RaycastAgainstZPlane( const Vec3& startPos, const Vec3& forwardNormal, float maxDist, float height ) const;
	RaycastResult RaycastAgainstWalls( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;
	RaycastResult RaycastAgainstEntities( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;
	RaycastResult RaycastAgainstEntitiesFast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;
	bool DoesRayHitEntityAlongZ( RaycastResult& raycastResult, const Vec3& potentialImpactPos, const Entity& entity ) const;

private:
	void				PopulateTiles( const std::vector<MapRegionTypeDefinition*>& regionTypeDefs );
	void				CreateInitialTiles( const std::vector<MapRegionTypeDefinition*>& regionTypeDefs );

	void				AddTileFace( const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topLeft, const Vec3& topRight, const Vec2& uvMins = Vec2::ZERO, const Vec2& uvMaxs = Vec2::ONE );

	// Tile helpers
	bool				IsAdjacentTileSolid( const Tile& tile, eCardinalDirection direction ) const;
	bool				IsTileSolid( int xCoord, int yCoord ) const;
	int					GetTileIndexFromTileCoords( int xCoord, int yCoord ) const;
	int					GetTileIndexFromTileCoords( const IntVec2& coords ) const;
	int					GetTileIndexFromWorldCoords( const Vec2& coords ) const;

	const Tile*			GetTileFromTileCoords( const IntVec2& tileCoords ) const;
	const Tile*			GetTileFromTileCoords( int xCoord, int yCoord ) const;
	const Tile*			GetTileFromWorldCoords( const Vec2& worldCoords ) const;
	const Vec2			GetWorldCoordsFromTile( const Tile& tile ) const;

	std::vector<const Tile*>	GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile ) const;
	
	void				BuildCardinalDirectionsArray();

	void				ResolveEntityVsWallCollisions();
	void				ResolveEntityVsWallCollision( Entity& entity );

private:
	Transform			m_raytraceTransform;

	std::vector<Tile>	m_tiles;
	IntVec2				m_dimensions;

	Vec2				m_cardinalDirectionOffsets[9];

	// For cube tests
	GPUMesh* m_cubeMesh = nullptr;
	Material* m_testMaterial = nullptr;

	std::vector<Transform> m_cubeMeshTransforms;

	std::vector<Vertex_PCUTBN> m_mesh;
};
