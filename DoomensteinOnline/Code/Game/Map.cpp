#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedStrings.hpp"
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
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const MapData& mapData, World* world )
	: m_world( world )
	, m_name( mapData.mapName )
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
	UpdateMeshes();
	ResolveEntityVsPortalCollisions();
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
		Entity*const& entity = m_entities[entityIdx];
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
	std::string entityTypeName = entityDefName;
	// Special case to handle spawning a new player
	if ( entityTypeName == "Player" )
	{
		entityTypeName = g_gameConfigBlackboard.GetValue( "playerEntityType", "" );
	}

	EntityDefinition* entityDef = EntityDefinition::GetEntityDefinition( entityTypeName );
	if ( entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Tried to spawn unrecognized entity '%s'", entityTypeName.c_str() ) );
		return nullptr;
	}

	return SpawnNewEntityOfType( *entityDef );
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( const EntityDefinition& entityDef )
{
	Entity* newEntity = nullptr;

	switch ( entityDef.GetClass() )
	{
		case eEntityClass::ACTOR:
		{
			newEntity = new Actor( entityDef );
			m_entities.push_back( newEntity );
		}
		break;

		case eEntityClass::PROJECTILE:
		{
			newEntity = new Projectile( entityDef );
			m_entities.push_back( newEntity );
		}
		break;

		case eEntityClass::PORTAL:
		{
			newEntity = new Portal( entityDef );
			m_entities.push_back( newEntity );
			m_portals.push_back( (Portal*)newEntity );
		}
		break;

		case eEntityClass::ENTITY:
		{
			newEntity = new Entity( entityDef );
			m_entities.push_back( newEntity );
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Tried to spawn entity '%s' with unknown type", entityDef.GetName().c_str() ) );
		}
	}

	if ( m_world != nullptr )
	{
		m_world->AddEntity( newEntity );
	}
	
	return newEntity;
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

		if ( mapEntityDef.entityDef->GetClass() == eEntityClass::PORTAL )
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
				 || entity->GetClass() == eEntityClass::PORTAL )
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
void Map::WarpEntityInMap( Entity* entity, Portal* portal )
{
	g_game->WarpToMap( entity, portal->GetDestinationMap(), portal->GetDestinationPosition(), entity->GetOrientationDegrees() + portal->GetDestinationYawOffset() );
}
