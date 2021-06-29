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
#include "Game/Portal.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapData.hpp"
#include "Game/World.hpp"


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
		/*DebugAddWorldWireCylinder( Vec3( entity->GetPosition(), 0.f ), Vec3( entity->GetPosition(), entity->GetHeight() ), 
								   entity->GetPhysicsRadius(), Rgba8::CYAN );*/
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

	Entity* newEntity = SpawnNewEntityOfType( *entityDef );
	newEntity->CreateZephyrScript( *entityDef );

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( const EntityDefinition& entityDef )
{
	Entity* entity = new Entity( entityDef, this );
	m_entities.emplace_back( entity );
	return entity;

	//switch ( entityDef.GetClass() )
	//{
	//	/*case eEntityClass::ACTOR:
	//	{
	//		Actor* actor = new Actor( entityDef );
	//		m_entities.emplace_back( actor );
	//		return actor;
	//	}
	//	break;

	//	case eEntityClass::PROJECTILE:
	//	{
	//		Projectile* projectile = new Projectile( entityDef );
	//		m_entities.emplace_back( projectile );
	//		return projectile;
	//	}
	//	break;

	//	case eEntityClass::PORTAL:
	//	{
	//		Portal* portal = new Portal( entityDef );
	//		m_entities.emplace_back( portal );
	//		m_portals.emplace_back( portal );
	//		return portal;
	//	}
	//	break;*/

	//	//case eEntityClass::ENTITY:
	//	//{
	//		Entity* entity = new Entity( entityDef, this );
	//		m_entities.emplace_back( entity );
	//		return entity;
	//	//}
	//	//break;

	//	/*default:
	//	{
	//		g_devConsole->PrintError( Stringf( "Tried to spawn entity '%s' with unknown type", entityDef.GetName().c_str() ) );
	//		return nullptr;
	//	}*/
	//}
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
void Map::InitializeAllZephyrEntityVariables()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->InitializeZephyrEntityVariables();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::CallAllMapEntityZephyrSpawnEvents( Entity* player )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity == player )
		{
			continue;
		}

		entity->FireSpawnEvent();
	}
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

		// Must be saved before initializing zephyr script
		newEntity->SetName( mapEntityDef.name );
		m_world->SaveEntityByName( newEntity );

		newEntity->CreateZephyrScript( *mapEntityDef.entityDef );

		newEntity->SetPosition( mapEntityDef.position );
		newEntity->SetOrientationDegrees( mapEntityDef.yawDegrees );

		// Define initial script values defined in map file
		// Note: These will override any initial values already defined in the EntityDefinition
		newEntity->InitializeScriptValues( mapEntityDef.zephyrScriptInitialValues );
		newEntity->SetEntityVariableInitializers( mapEntityDef.zephyrEntityVarInits );
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
