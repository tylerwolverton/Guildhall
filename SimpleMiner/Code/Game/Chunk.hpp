#pragma once
#include "Game/Block.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


//-----------------------------------------------------------------------------------------------
class Chunk
{
public:
	Chunk( const IntVec2& worldCoords, const AABB3& worldBounds );
	~Chunk();

	void Render() const;
	void DebugRender() const;

private:
	void RebuildMesh();
	void PushBlockFaces( int blockIdx );

private:
	Block m_blocks[NUM_BLOCKS_IN_CHUNK];

	IntVec2 m_worldCoords;
	AABB3 m_worldBounds;

	std::vector<Vertex_PCU> m_vertices;
};
