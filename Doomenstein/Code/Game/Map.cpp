#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Actor.hpp"
#include "Game/Projectile.hpp"
#include "Game/Portal.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( std::string name, MapDefinition* mapDef )
	: m_name( name )
	, m_mapDef( mapDef )
{
	LoadEntitiesFromDefinition();
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		DebugAddWorldWireCylinder( Vec3( entity->GetPosition(), 0.f ), Vec3( entity->GetPosition(), entity->GetHeight() ), 
								   entity->GetPhysicsRadius(), Rgba8::CYAN );
	}
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( const std::string& entityDefName )
{
	EntityDefinition* entityDef = EntityDefinition::GetEntityDefinition( entityDefName );
	if ( entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Tried to spawn unrecognized entity '%s'", entityDefName.c_str() ) );
		return nullptr;
	}

	return SpawnNewEntityOfType( *entityDef );
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( const EntityDefinition& entityDef )
{
	if ( entityDef.GetType() == "Entity" )
	{
		Entity* entity = new Entity( entityDef );
		m_entities.emplace_back( entity );
		return entity;
	}
	else if ( entityDef.GetType() == "Actor" )
	{
		Actor* actor = new Actor( entityDef );
		m_entities.emplace_back( actor );
		return actor;
	}
	else if ( entityDef.GetType() == "Projectile" )
	{
		Projectile* projectile = new Projectile( entityDef );
		m_entities.emplace_back( projectile );
		return projectile;
	}
	else if ( entityDef.GetType() == "Portal" )
	{
		Portal* portal = new Portal( entityDef );
		m_entities.emplace_back( portal );
		return portal;
	}
	
	g_devConsole->PrintError( Stringf( "Tried to spawn entity with unknown type '%s'", entityDef.GetType().c_str() ) );
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	float maxDistToSearch = maxDist;
	Entity* closestEntity = nullptr;

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( IsPointInForwardSector2D( entity->GetPosition(), observerPos, forwardDegrees, apertureDegrees, maxDistToSearch ) )
		{
			closestEntity = entity;
			maxDistToSearch = GetDistance2D( observerPos, entity->GetPosition() );
		}
	}

	return closestEntity;
}


//-----------------------------------------------------------------------------------------------
void Map::LoadEntitiesFromDefinition()
{
	std::vector<MapEntityDefinition> mapEntityDefs = m_mapDef->GetMapEntityDefs();

	for ( int mapEntityIdx = 0; mapEntityIdx < (int)mapEntityDefs.size(); ++mapEntityIdx )
	{
		MapEntityDefinition& mapEntityDef = mapEntityDefs[mapEntityIdx];
		if ( mapEntityDef.entityDef == nullptr )
		{
			continue;
		}

		Entity* newEntity = SpawnNewEntityOfType( *mapEntityDef.entityDef );
		if ( newEntity == nullptr )
		{
			continue;
		}

		newEntity->SetPosition( mapEntityDef.position );
		newEntity->SetOrientationDegrees( mapEntityDef.yawDegrees );
	}

}
