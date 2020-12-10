#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/Entity.hpp"
#include "Game/Actor.hpp"
#include "Game/Projectile.hpp"
#include "Game/Portal.hpp"
#include "Game/Pickup.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapData.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const MapData& mapData, World* world )
	: m_name( mapData.mapName )
	, m_playerStartPos( mapData.playerStartPos )
	, m_playerStartYaw( mapData.playerStartYaw )
	, m_world( world )
{
	LoadEntities( mapData.mapEntityDefs );
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] != nullptr
			 && m_entities[entityIdx]->GetOwner() == this )
		{
			m_entities[entityIdx]->Die();

			PTR_SAFE_DELETE( m_entities[entityIdx] );
		}
	}

	m_entities.clear();
}


//-----------------------------------------------------------------------------------------------
void Map::Load( Entity* player )
{
	if ( player == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map '%s': Tried to load a map with a null player", m_name.c_str() ) );
		return;
	}

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] != nullptr )
		{
			m_entities[entityIdx]->Load();
		}
	}
	
	m_player = player;
	m_entities.push_back( player );

	player->SetMap( this );
	player->SetOwner( this );
	player->SetPosition( m_playerStartPos );
	player->SetOrientationDegrees( m_playerStartYaw );
	player->FireSpawnEvent();
}


//-----------------------------------------------------------------------------------------------
void Map::Unload()
{
	// Remove reference to player from this map
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] == m_player )
		{
			m_entities[entityIdx] = nullptr;
			continue;
		}

		if ( m_entities[entityIdx] != nullptr 
			 && m_entities[entityIdx]->GetOwner() == this )
		{
			m_entities[entityIdx]->Unload();
		}
	}

	m_player = nullptr;
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

	UpdateMesh();

	DeleteDeadEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Render();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->DebugRender();
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
	switch ( entityDef.GetClass() )
	{
		case eEntityClass::ACTOR:
		{
			Actor* actor = new Actor( entityDef, this );
			AddToEntityList( actor );
			return actor;
		}
		break;

		case eEntityClass::PROJECTILE:
		{
			Projectile* projectile = new Projectile( entityDef, this );
			AddToEntityList( projectile );
			return projectile;
		}
		break;

		case eEntityClass::PORTAL:
		{
			Portal* portal = new Portal( entityDef, this );
			AddToEntityList( portal );
			return portal;
		}
		break;

		case eEntityClass::PICKUP:
		{
			Pickup* pickup = new Pickup( entityDef, this );
			AddToEntityList( pickup );
			return pickup;
		}
		break;

		case eEntityClass::ENTITY:
		{
			Entity* entity = new Entity( entityDef, this );
			AddToEntityList( entity );
			return entity;
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Tried to spawn entity '%s' with unknown type", entityDef.GetType().c_str() ) );
			return nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfTypeAtPosition( const std::string& entityDefName, const Vec2& pos )
{
	Entity* entity = SpawnNewEntityOfType( entityDefName );
	entity->SetPosition( pos );

	return entity;
}


//-----------------------------------------------------------------------------------------------
void Map::UnloadAllEntityScripts()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->UnloadZephyrScript();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ReloadAllEntityScripts()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->ReloadZephyrScript();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveOwnershipOfEntity( Entity* entityToRemove )
{

	//entityToAdd->SetOwner( this );
}


//-----------------------------------------------------------------------------------------------
void Map::TakeOwnershipOfEntity( Entity* entityToAdd )
{
	if ( entityToAdd == nullptr )
	{
		return;
	}

	entityToAdd->SetMap( this );
	entityToAdd->SetOwner( this );
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveEntityFromMap( Entity* entityToRemove )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity == entityToRemove )
		{
			m_entities[entityIdx] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::AddEntityToMap( Entity* entityToAdd )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			entity = entityToAdd;
			return;
		}
	}

	m_entities.push_back( entityToAdd );
}


//-----------------------------------------------------------------------------------------------
void Map::AddItemToTargetInventory( Entity* item, Entity* targetEntity )
{
	if ( targetEntity == nullptr
		 || item == nullptr )
	{
		return;
	}

	targetEntity->AddItemToInventory( item );

	item->m_rigidbody2D->Disable();

	RemoveOwnershipOfEntity( item );
}


//-----------------------------------------------------------------------------------------------
void Map::LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs )
{
	for ( int mapEntityIdx = 0; mapEntityIdx < (int)mapEntityDefs.size(); ++mapEntityIdx )
	{
		const MapEntityDefinition& mapEntityDef = mapEntityDefs[mapEntityIdx];
		if ( mapEntityDef.entityDef == nullptr )
		{
			continue;
		}

		Entity* newEntity = SpawnNewEntityOfType( *mapEntityDef.entityDef );
		if ( newEntity == nullptr )
		{
			continue;
		}

		newEntity->SetName( mapEntityDef.name );
		newEntity->SetPosition( mapEntityDef.position );
		newEntity->SetOrientationDegrees( mapEntityDef.yawDegrees );

		if ( mapEntityDef.entityDef->GetClass() == eEntityClass::PORTAL )
		{
			Portal* portal = (Portal*)newEntity;
			portal->SetDestinationMap( mapEntityDef.portalDestMap );
			portal->SetDestinationPosition( mapEntityDef.portalDestPos );
			portal->SetDestinationYawOffset( mapEntityDef.portalDestYawOffset );
		}

		// Define initial script values defined in map file
		// Note: These will override any initial values already defined in the EntityDefinition
		newEntity->InitializeScriptValues( mapEntityDef.zephyrScriptInitialValues );
		newEntity->FireSpawnEvent();

		m_world->SaveEntityByName( newEntity );
	}

}


//-----------------------------------------------------------------------------------------------
void Map::AddToEntityList( Entity* entity )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] == nullptr )
		{
			m_entities[entityIdx] = entity;
			return;
		}
	}

	m_entities.push_back( entity );
}


//-----------------------------------------------------------------------------------------------
void Map::DeleteDeadEntities()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || !entity->IsDead() )
		{
			continue;
		}
		
		delete( m_entities[entityIdx] );
		m_entities[entityIdx] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::WarpEntityInMap( Entity* entity, Portal* portal )
{
	g_game->WarpToMap( entity, portal->GetDestinationMap(), portal->GetDestinationPosition(), entity->GetOrientationDegrees() + portal->GetDestinationYawOffset() );
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityByName( const std::string& name )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
		{
			continue;
		}
		
		if ( entity->GetName() == name )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityById( EntityId id )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
		{
			continue;
		}

		if ( entity->GetId() == id )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityAtPosition( const Vec2& position )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
		{
			continue;
		}

		if ( IsPointInsideDisc( position, entity->GetPosition(), entity->GetPhysicsRadius() ) )
		{
			return entity;
		}
	}

	return nullptr;
}
