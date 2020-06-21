#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

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
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_VECTOR_SAFE_DELETE( m_entities );
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
	if ( entityDef.GetType() == "entity" )
	{
		Entity* entity = new Entity( entityDef );
		m_entities.emplace_back( entity );
		return entity;
	}
	else if ( entityDef.GetType() == "actor" )
	{
		Actor* actor = new Actor( entityDef );
		m_entities.emplace_back( actor );
		return actor;
	}
	else if ( entityDef.GetType() == "projectile" )
	{
		Projectile* projectile = new Projectile( entityDef );
		m_entities.emplace_back( projectile );
		return projectile;
	}
	else if ( entityDef.GetType() == "portal" )
	{
		Portal* portal = new Portal( entityDef );
		m_entities.emplace_back( portal );
		return portal;
	}
	
	g_devConsole->PrintError( Stringf( "Tried to spawn entity with unknown type '%s'", entityDef.GetType().c_str() ) );
	return nullptr;
}
