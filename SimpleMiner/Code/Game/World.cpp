#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
World::World()
{
	m_chunks.emplace_back( IntVec2::ZERO, AABB3( 0.f, 0.f, 0.f, (float)CHUNK_WIDTH, (float)CHUNK_LENGTH, (float)CHUNK_HEIGHT ) );
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
}


//-----------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds )
{
}


//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	for ( int chunkIdx = 0; chunkIdx < (int)m_chunks.size(); ++chunkIdx )
	{
		m_chunks[chunkIdx].Render();
	}
}


//-----------------------------------------------------------------------------------------------
void World::DebugRender() const
{
	for ( int chunkIdx = 0; chunkIdx < (int)m_chunks.size(); ++chunkIdx )
	{
		m_chunks[chunkIdx].DebugRender();
	}
}
