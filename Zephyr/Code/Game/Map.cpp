#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/Actor.hpp"
#include "Game/Projectile.hpp"
#include "Game/Portal.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapData.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const MapData& mapData )
	: m_name( mapData.mapName )
	, m_playerStartPos( mapData.playerStartPos )
	, m_playerStartYaw( mapData.playerStartYaw )
{
	LoadEntities( mapData.mapEntityDefs );
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::Load( Entity* player )
{
	m_player = player;
	m_entities.push_back( player );

	player->SetMap( this );
	player->SetPosition( m_playerStartPos );
	player->SetOrientationDegrees( m_playerStartYaw );
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

	ResolveEntityVsEntityCollisions();
	UpdateMesh();
	ResolveEntityVsPortalCollisions();

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
	switch ( entityDef.GetType() )
	{
		case eEntityType::ACTOR:
		{
			Actor* actor = new Actor( entityDef, this );
			AddToEntityList( actor );
			return actor;
		}
		break;

		case eEntityType::PROJECTILE:
		{
			Projectile* projectile = new Projectile( entityDef, this );
			AddToEntityList( projectile );
			AddToProjectileList( projectile );
			return projectile;
		}
		break;

		case eEntityType::PORTAL:
		{
			Portal* portal = new Portal( entityDef, this );
			AddToEntityList( portal );
			AddToPortalList( portal );
			return portal;
		}
		break;

		case eEntityType::ENTITY:
		{
			Entity* entity = new Entity( entityDef, this );
			AddToEntityList( entity );
			return entity;
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Tried to spawn entity '%s' with unknown type", entityDef.GetName().c_str() ) );
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
void Map::RemoveOwnershipOfEntity( Entity* entityToRemove )
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
void Map::TakeOwnershipOfEntity( Entity* entityToAdd )
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

		newEntity->SetPosition( mapEntityDef.position );
		newEntity->SetOrientationDegrees( mapEntityDef.yawDegrees );

		if ( mapEntityDef.entityDef->GetType() == eEntityType::PORTAL )
		{
			Portal* portal = (Portal*)newEntity;
			portal->SetDestinationMap( mapEntityDef.portalDestMap );
			portal->SetDestinationPosition( mapEntityDef.portalDestPos );
			portal->SetDestinationYawOffset( mapEntityDef.portalDestYawOffset );
		}
	}

}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityVsEntityCollisions()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		for ( int otherEntityIdx = entityIdx + 1; otherEntityIdx < (int)m_entities.size(); ++otherEntityIdx )
		{
			Entity* const& otherEntity = m_entities[otherEntityIdx];
			if ( otherEntity == nullptr )
			{
				continue;
			}

			ResolveEntityVsEntityCollision( *entity, *otherEntity );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityVsEntityCollision( Entity& entity1, Entity& entity2 )
{
	// Neither can be moved
	if ( !entity1.m_canBePushedByEntities
		 && !entity2.m_canBePushedByEntities )
	{
		return;
	}

	// Neither can push
	if ( !entity1.m_canPushEntities
		 && !entity2.m_canPushEntities )
	{
		return;
	}

	float radius1 = entity1.GetPhysicsRadius();
	float radius2 = entity2.GetPhysicsRadius();

	// Both can be moved
	if ( entity1.m_canBePushedByEntities
		 && entity2.m_canBePushedByEntities )
	{
		if ( entity1.m_canPushEntities
			 && entity2.m_canPushEntities )
		{
			PushDiscsOutOfEachOtherRelativeToMass2D( entity1.m_position, radius1, entity1.GetMass(), entity2.m_position, radius2, entity2.GetMass() );
		}

		if ( entity1.m_canPushEntities
			 && !entity2.m_canPushEntities )
		{
			PushDiscOutOfDisc2D( entity2.m_position, radius2, entity1.m_position, radius1 );
		}

		if ( entity2.m_canPushEntities
			 && !entity1.m_canPushEntities )
		{
			PushDiscOutOfDisc2D( entity1.m_position, radius1, entity2.m_position, radius2 );
		}
	}

	// Only entity1 can be moved
	if ( entity1.m_canBePushedByEntities
		 && !entity2.m_canBePushedByEntities )
	{
		if ( entity2.m_canPushEntities )
		{
			PushDiscOutOfDisc2D( entity1.m_position, radius1, entity2.m_position, radius2 );
		}
	}

	// Only entity2 can be moved
	if ( entity2.m_canBePushedByEntities
		 && !entity1.m_canBePushedByEntities )
	{
		if ( entity1.m_canPushEntities )
		{
			PushDiscOutOfDisc2D( entity2.m_position, radius2, entity1.m_position, radius1 );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityVsPortalCollisions()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		for ( int portalIdx = 0; portalIdx < (int)m_portals.size(); ++portalIdx )
		{
			Portal* const& portal = m_portals[portalIdx];
			if ( portal == nullptr
				 || entity->GetType() == eEntityType::PORTAL )
			{
				continue;
			}

			if ( DoDiscsOverlap( entity->GetPosition(), entity->GetPhysicsRadius(), portal->GetPosition(), portal->GetPhysicsRadius() ) )
			{
				WarpEntityInMap( entity, portal );
			}
		}
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
void Map::AddToProjectileList( Projectile* projectile )
{
	for ( int projectileIdx = 0; projectileIdx < (int)m_projectiles.size(); ++projectileIdx )
	{
		if ( m_projectiles[projectileIdx] == nullptr )
		{
			m_projectiles[projectileIdx] = projectile;
			return;
		}
	}

	m_projectiles.push_back( projectile );
}


//-----------------------------------------------------------------------------------------------
void Map::AddToPortalList( Portal* portal )
{
	for ( int portalIdx = 0; portalIdx < (int)m_portals.size(); ++portalIdx )
	{
		if ( m_portals[portalIdx] == nullptr )
		{
			m_portals[portalIdx] = portal;
			return;
		}
	}

	m_portals.push_back( portal );
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveFromProjectileList( Projectile* projectile )
{
	for ( int projectileIdx = 0; projectileIdx < (int)m_projectiles.size(); ++projectileIdx )
	{
		if ( projectile == m_projectiles[projectileIdx] )
		{
			m_projectiles[projectileIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveFromPortalList( Portal* portal )
{
	for ( int portalIdx = 0; portalIdx < (int)m_portals.size(); ++portalIdx )
	{
		if ( portal == m_portals[portalIdx] )
		{
			m_portals[portalIdx] = nullptr;
			return;
		}
	}
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

		switch( entity->GetType() )
		{
			case eEntityType::PROJECTILE: RemoveFromProjectileList( (Projectile*)entity ); break;
			case eEntityType::PORTAL: RemoveFromPortalList( (Portal*)entity ); break;
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
