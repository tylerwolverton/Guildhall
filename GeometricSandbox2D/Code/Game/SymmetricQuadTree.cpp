#include "Game/SymmetricQuadTree.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/GameCommon.hpp"

#include <queue>

//-----------------------------------------------------------------------------------------------
SymmetricQuadTree::SymmetricQuadTree( const AABB2& topLevelBoundingBox, int numTreeLevels )
	: m_numTreeLevels( numTreeLevels )
{
	ReserveVectorForTotalTreeNodes();
	
	Node rootNode( topLevelBoundingBox, 0, 0 );

	SplitIntoQuadrants( rootNode );
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::DebugRender( std::vector<Vertex_PCU>& vertices ) const
{
	Rgba8 outlineColor = Rgba8::MAGENTA;
	outlineColor.a = 100;

	Rgba8 highlightColor = Rgba8::WHITE;
	highlightColor.a = 200;

	for ( int nodeIdx = 0; nodeIdx < (int)m_treeNodes.size(); ++nodeIdx )
	{
		AppendVertsForAABB2Outline( vertices, m_treeNodes[nodeIdx].sectorBoundingBox, outlineColor, g_debugLineThickness );

		if ( m_treeNodes[nodeIdx].entityIds.size() > 0 )
		{
			AppendVertsForAABB2D( vertices, m_treeNodes[nodeIdx].sectorBoundingBox, highlightColor );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::DebugRenderRaycast( std::vector<Vertex_PCU>& vertices, const Vec2& rayStart, const Vec2& rayForwardNormal, float rayMaxLength ) const
{
	Rgba8 highlightColor = Rgba8::ORANGE;
	highlightColor.a = 200;

	for ( int nodeIdx = 0; nodeIdx < (int)m_treeNodes.size(); ++nodeIdx )
	{
		Node const& node = m_treeNodes[nodeIdx];

		if ( node.index == -1 && DoLineSegmentAndAABBOverlap2D( rayStart, rayForwardNormal, rayMaxLength, node.sectorBoundingBox ) )
		{
			AppendVertsForAABB2D( vertices, node.sectorBoundingBox, highlightColor );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::AddEntities( std::vector<Entity*> entities )
{
	for ( int entityIdx = 0; entityIdx < (int)entities.size(); ++entityIdx )
	{
		AddEntity( entities[entityIdx] );
	}
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::AddEntity( Entity* entity )
{
	if ( m_treeNodes.size() == 0 )
	{
		g_devConsole->PrintError( "QuadTree is empty, cannot add entity" );
		return;
	}

	AddEntityToOverlappingNode( m_treeNodes[0], entity );
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::RemoveEntitiesAboveId( EntityId maxIdInclusive )
{
	// Take advantage of the fact that entities will always be added and removed in order to pop off the most recent entities
	for ( int nodeIdx = 0; nodeIdx < (int)m_treeNodes.size(); ++nodeIdx )
	{
		while ( !m_treeNodes[nodeIdx].entityIds.empty() && m_treeNodes[nodeIdx].entityIds.back() > maxIdInclusive )
		{
			m_treeNodes[nodeIdx].entityIds.pop_back();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::RemoveEntity( EntityId id )
{
	for ( int nodeIdx = 0; nodeIdx < (int)m_treeNodes.size(); ++nodeIdx )
	{
		Node& node = m_treeNodes[nodeIdx];

		int idxOfIdToRemove = -1;
		for ( int entityIdIdx = 0; entityIdIdx < (int)node.entityIds.size(); ++entityIdIdx )
		{
			if ( node.entityIds[entityIdIdx] == id )
			{
				idxOfIdToRemove = entityIdIdx;
				break;
			}
		}

		if ( idxOfIdToRemove != -1 )
		{
			node.entityIds.erase( node.entityIds.begin() + idxOfIdToRemove );
		}
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<EntityId> SymmetricQuadTree::GetEntitiesFromRaycast( const Vec2& rayStart, const Vec2& rayForwardNormal, float rayMaxLength )
{
	std::vector<EntityId> entityIds;
	AddEntitiesFromRaycast( entityIds, m_treeNodes[0], rayStart, rayForwardNormal, rayMaxLength );

	return entityIds;
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::AddEntitiesFromRaycast( std::vector<EntityId>& entityIds, const Node& nodeToTest, const Vec2& rayStart, const Vec2& rayForwardNormal, float rayMaxLength )
{
	// Stop testing this branch if there is no collision
	if ( !DoLineSegmentAndAABBOverlap2D( rayStart, rayForwardNormal, rayMaxLength, nodeToTest.sectorBoundingBox ) )
	{
		return;
	}

	// Add all entities from leaf node into entity list
	if ( nodeToTest.index == -1 )
	{
		for ( EntityId sectorEntity : nodeToTest.entityIds )
		{
			entityIds.push_back( sectorEntity );
		}

		return;
	}

	// The ray hit this sector and it isn't a leaf, so check children
	AddEntitiesFromRaycast( entityIds, GetChildNode( nodeToTest, eChildQuadrant::TOP_RIGHT ),	 rayStart, rayForwardNormal, rayMaxLength );
	AddEntitiesFromRaycast( entityIds, GetChildNode( nodeToTest, eChildQuadrant::TOP_LEFT ),	 rayStart, rayForwardNormal, rayMaxLength );
	AddEntitiesFromRaycast( entityIds, GetChildNode( nodeToTest, eChildQuadrant::BOTTOM_LEFT ),  rayStart, rayForwardNormal, rayMaxLength );
	AddEntitiesFromRaycast( entityIds, GetChildNode( nodeToTest, eChildQuadrant::BOTTOM_RIGHT ), rayStart, rayForwardNormal, rayMaxLength );
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::ReserveVectorForTotalTreeNodes()
{
	int totalNodeCount = 1;
	int lastLevelNodeCount = 1;
	for ( int treeLevel = 1; treeLevel <= m_numTreeLevels; ++treeLevel )
	{
		lastLevelNodeCount *= 4;
		totalNodeCount += lastLevelNodeCount;
	}

	m_treeNodes.reserve( totalNodeCount );
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::SplitIntoQuadrants( const Node& parentNode )
{
	std::deque<Node> nodesToProcess;
	nodesToProcess.push_back( parentNode );
	int nextNodeIdx = 0;

	while ( !nodesToProcess.empty() )
	{
		Node nextNode = nodesToProcess.front();
		nodesToProcess.pop_front();

		m_treeNodes.push_back( nextNode );

		if ( nextNode.index == -1 )
		{
			continue;
		}

		// Split into children
		Vec2 sectorCenter = nextNode.sectorBoundingBox.GetCenter();
		AABB2 sectorBounds = nextNode.sectorBoundingBox;
	
		AABB2 topRight0 =		AABB2( sectorCenter.x,		sectorCenter.y,			sectorBounds.maxs.x,	sectorBounds.maxs.y );
		AABB2 topLeft1 =		AABB2( sectorBounds.mins.x, sectorCenter.y,			sectorCenter.x,			sectorBounds.maxs.y );
		AABB2 bottomLeft2 =		AABB2( sectorBounds.mins.x, sectorBounds.mins.y,	sectorCenter.x,			sectorCenter.y );
		AABB2 bottomRight3 =	AABB2( sectorCenter.x,		sectorBounds.mins.y,	sectorBounds.maxs.x,	sectorCenter.y );
	
		int firstNodeIdx = ++nextNodeIdx;
		int secondNodeIdx = ++nextNodeIdx;
		int thirdNodeIdx = ++nextNodeIdx;
		int fourthNodeIdx = ++nextNodeIdx;
		
		// Handle leaf nodes
		if ( nextNode.treeLevel == m_numTreeLevels )
		{
			firstNodeIdx = secondNodeIdx = thirdNodeIdx = fourthNodeIdx = -1;
		}

		// Push back children to process
		Node topRightQuad0( topRight0, firstNodeIdx, nextNode.treeLevel + 1 );
		nodesToProcess.push_back( topRightQuad0 );

		Node topLeftQuad1( topLeft1, secondNodeIdx, nextNode.treeLevel + 1 );
		nodesToProcess.push_back( topLeftQuad1 );

		Node bottomLeftQuad2( bottomLeft2, thirdNodeIdx, nextNode.treeLevel + 1 );
		nodesToProcess.push_back( bottomLeftQuad2 );

		Node bottomRightQuad3( bottomRight3, fourthNodeIdx, nextNode.treeLevel + 1 );
		nodesToProcess.push_back( bottomRightQuad3 );
	}
}


//-----------------------------------------------------------------------------------------------
void SymmetricQuadTree::AddEntityToOverlappingNode( Node& parentNode, Entity* entity )
{
	bool entityOverlapsNode = DoDiscAndAABBOverlap2D( entity->GetPosition(), entity->GetPhysicsRadius(), parentNode.sectorBoundingBox );
	
	// No overlap, so no need to check children either 
	if ( !entityOverlapsNode )
	{
		return;
	}

	// This is a leaf node
	if ( parentNode.index == -1 )
	{
		parentNode.entityIds.push_back( entity->GetId() );
		return;
	}

	// Check children
	AddEntityToOverlappingNode( GetChildNode( parentNode, eChildQuadrant::TOP_RIGHT ), entity );
	AddEntityToOverlappingNode( GetChildNode( parentNode, eChildQuadrant::TOP_LEFT ), entity );
	AddEntityToOverlappingNode( GetChildNode( parentNode, eChildQuadrant::BOTTOM_LEFT ), entity );
	AddEntityToOverlappingNode( GetChildNode( parentNode, eChildQuadrant::BOTTOM_RIGHT ), entity );
}


//-----------------------------------------------------------------------------------------------
Node& SymmetricQuadTree::GetChildNode( const Node& parentNode, eChildQuadrant childQuadrant )
{
	// Check if this is a leaf and return the root node to signal no children
	if ( parentNode.index == -1 )
	{
		return m_treeNodes[0];
	}

	int firstChldIdx = ( parentNode.index * 4 ) + 1;

	GUARANTEE_OR_DIE( firstChldIdx + 3 <= (int)m_treeNodes.size() - 1, "Child index will be out of bounds, quad tree is malformed" );

	switch ( childQuadrant )
	{
		case eChildQuadrant::TOP_RIGHT:		return m_treeNodes[firstChldIdx];
		case eChildQuadrant::TOP_LEFT:		return m_treeNodes[firstChldIdx + 1];
		case eChildQuadrant::BOTTOM_LEFT:	return m_treeNodes[firstChldIdx + 2];
		case eChildQuadrant::BOTTOM_RIGHT:	return m_treeNodes[firstChldIdx + 3];
	}

	// Impossible to get here
	return m_treeNodes[0];
}


//-----------------------------------------------------------------------------------------------
const Node& SymmetricQuadTree::GetChildNode( const Node& parentNode, eChildQuadrant childQuadrant ) const
{
	// Check if this is a leaf and return the root node to signal no children
	if ( parentNode.index == -1 )
	{
		return m_treeNodes[0];
	}

	int firstChldIdx = ( parentNode.index * 4 ) + 1;

	GUARANTEE_OR_DIE( firstChldIdx + 3 <= (int)m_treeNodes.size() - 1, "Child index will be out of bounds, quad tree is malformed" );

	switch ( childQuadrant )
	{
		case eChildQuadrant::TOP_RIGHT:		return m_treeNodes[firstChldIdx];
		case eChildQuadrant::TOP_LEFT:		return m_treeNodes[firstChldIdx + 1];
		case eChildQuadrant::BOTTOM_LEFT:	return m_treeNodes[firstChldIdx + 2];
		case eChildQuadrant::BOTTOM_RIGHT:	return m_treeNodes[firstChldIdx + 3];
	}

	// Impossible to get here
	return m_treeNodes[0];
}


