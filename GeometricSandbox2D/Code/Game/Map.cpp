#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const std::string& name )
	: m_name( name )
{
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::Load()
{
}


//-----------------------------------------------------------------------------------------------
void Map::Unload()
{
}


//-----------------------------------------------------------------------------------------------
void Map::Reset()
{
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Update( deltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	std::vector<Vertex_PCU> vertices;
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Render( vertices );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	std::vector<Vertex_PCU> vertices;
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->DebugRender( vertices );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::AddEntitySet( int numEntities )
{
	UNUSED( numEntities );
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveEntitySet( int numEntities )
{
	UNUSED( numEntities );
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntity( const Polygon2& polygon )
{
	Entity* newEntity = new Entity( polygon );
	m_entities.push_back( newEntity );

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
int Map::GetObjectCount() const
{
	return 0;
}


//-----------------------------------------------------------------------------------------------
int Map::GetRaycastCount() const
{
	return 0;
}


//-----------------------------------------------------------------------------------------------
void Map::CycleBroadphaseCheck()
{
	switch ( m_broadphaseCheckType )
	{
		case eBroadphaseCheckType::NONE: m_broadphaseCheckType = eBroadphaseCheckType::QUAD_TREE; return;
		case eBroadphaseCheckType::QUAD_TREE: m_broadphaseCheckType = eBroadphaseCheckType::NONE; return;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::CycleNarrowphaseCheck()
{
	switch ( m_narrowphaseCheckType )
	{
		case eNarrowphaseCheckType::NONE: m_narrowphaseCheckType = eNarrowphaseCheckType::BOUNDING_DISC; return;
		case eNarrowphaseCheckType::BOUNDING_DISC: m_narrowphaseCheckType = eNarrowphaseCheckType::NONE; return;
	}
}


//-----------------------------------------------------------------------------------------------
std::string Map::GetBroadphaseCheckTypeStr() const
{
	switch ( m_broadphaseCheckType )
	{
		case eBroadphaseCheckType::NONE: return "None";;
		case eBroadphaseCheckType::QUAD_TREE: return "Quad Tree";
	}

	return "None";
}


//-----------------------------------------------------------------------------------------------
std::string Map::GetNarowphaseCheckTypeStr() const
{
	switch ( m_narrowphaseCheckType )
	{
		case eNarrowphaseCheckType::NONE: return "None";;
		case eNarrowphaseCheckType::BOUNDING_DISC: return "Bounding Disc";
	}

	return "None";
}


//-----------------------------------------------------------------------------------------------
RaycastResult Map::Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist )
{
	UNUSED( startPos );
	UNUSED( forwardNormal );
	UNUSED( maxDist );

	return RaycastResult();
}


