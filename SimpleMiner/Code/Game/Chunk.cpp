#include "Game/Chunk.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Game.hpp"

//-----------------------------------------------------------------------------------------------
Chunk::Chunk( const IntVec2& worldCoords, const AABB3& worldBounds )
	: m_worldCoords( worldCoords )
	, m_worldBounds( worldBounds )
{
	// Randomize blocks
	for ( int blockIdx = 0; blockIdx < NUM_BLOCKS_IN_CHUNK; ++blockIdx )
	{
		m_blocks[blockIdx].SetType( (unsigned char)g_game->m_rng->RollRandomIntLessThan( 2 ) );
	}

	RebuildMesh();
}


//-----------------------------------------------------------------------------------------------
Chunk::~Chunk()
{
}


//-----------------------------------------------------------------------------------------------
void Chunk::Render() const
{
	g_renderer->DrawVertexArray<Vertex_PCU>( m_vertices );
}


//-----------------------------------------------------------------------------------------------
void Chunk::DebugRender() const
{
	DebugAddWorldWireBounds( m_worldBounds, Rgba8::WHITE );
}


//-----------------------------------------------------------------------------------------------
void Chunk::RebuildMesh()
{
	m_vertices.clear();

	for ( int blockIdx = 0; blockIdx < NUM_BLOCKS_IN_CHUNK; ++blockIdx )
	{
		PushBlockFaces( blockIdx );
	}
}


//-----------------------------------------------------------------------------------------------
void Chunk::PushBlockFaces( int blockIdx )
{
	// Bottom
	m_vertices.emplace_back( Vec3( 0.f, 0.f, 0.f ), Rgba8::WHITE, Vec2::ZERO );
	m_vertices.emplace_back( Vec3( 0.f, 1.f, 0.f ), Rgba8::WHITE, Vec2::ZERO_TO_ONE );
	m_vertices.emplace_back( Vec3( 1.f, 1.f, 0.f ), Rgba8::WHITE, Vec2::ONE );
	m_vertices.emplace_back( Vec3( 1.f, 0.f, 0.f ), Rgba8::WHITE, Vec2::ONE_ZERO );
}
