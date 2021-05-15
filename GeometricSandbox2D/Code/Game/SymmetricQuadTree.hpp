#pragma once
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct AABB2;


//-----------------------------------------------------------------------------------------------
enum class eChildQuadrant
{
	TOP_RIGHT,
	TOP_LEFT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};


//-----------------------------------------------------------------------------------------------
struct Node
{
public:
	int index = -1;
	int treeLevel = -1;
	AABB2 sectorBoundingBox;
	std::vector<EntityId> entityIds;

public:
	Node() {}
	Node( const AABB2& sectorBoundingBox )
		: sectorBoundingBox( sectorBoundingBox )
	{}

	Node( const AABB2& sectorBoundingBox, int index )
		: sectorBoundingBox( sectorBoundingBox )
		, index( index )
	{}

	Node( const AABB2& sectorBoundingBox, int index, int treeLevel )
		: sectorBoundingBox( sectorBoundingBox )
		, index( index )
		, treeLevel( treeLevel )
	{}
};


//-----------------------------------------------------------------------------------------------
class SymmetricQuadTree
{
public:
	SymmetricQuadTree( const AABB2& topLevelBoundingBox, int numTreeLevels );
	~SymmetricQuadTree() = default;

	void DebugRender( std::vector<Vertex_PCU>& vertices ) const;
	void DebugRenderRaycast( std::vector<Vertex_PCU>& vertices, const Vec2& rayStart, const Vec2& rayForwardNormal, float rayMaxLength ) const;

	void AddEntities( std::vector<Entity*> entities );
	void AddEntity( Entity* entity );
	void RemoveEntitiesAboveId( EntityId maxIdInclusive );
	void RemoveEntity( EntityId id );

	std::vector<EntityId> GetEntitiesFromRaycast( const Vec2& rayStart, const Vec2& rayForwardNormal, float rayMaxLength );
	void AddEntitiesFromRaycast( std::vector<EntityId>& entityIds, const Node& nodeToTest, const Vec2& rayStart, const Vec2& rayForwardNormal, float rayMaxLength );

private:
	void ReserveVectorForTotalTreeNodes();

	void SplitIntoQuadrants( const Node& parentNode );

	void AddEntityToOverlappingNode( Node& parentNode, Entity* entity );

	Node& GetChildNode( const Node& parentNode, eChildQuadrant childQuadrant );
	const Node& GetChildNode( const Node& parentNode, eChildQuadrant childQuadrant ) const;

private:
	std::vector<Node>	m_treeNodes;
	int					m_numTreeLevels = 0;
};
