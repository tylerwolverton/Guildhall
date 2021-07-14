#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapData.hpp"
#include "Game/PhysicsConfig.hpp"
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
	// TODO: Fire leave events for each collision entity
	entityToRemove->m_collidingEntities.clear();

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

			if ( !g_physicsConfig->DoLayersInteract( entity->GetCollisionLayer(), otherEntity->GetCollisionLayer() ) )
			{
				continue;
			}

			ResolveCollisionEvents( entity, otherEntity );

			// Account for the case where a collision event removes an entity from the map
			if ( entity == nullptr
				 || otherEntity == nullptr )
			{
				continue;
			}

			ResolveEntityVsEntityCollision( *entity, *otherEntity );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveCollisionEvents( Entity* entity, Entity* otherEntity )
{
	EventArgs args;
	args.SetValue( "otherEntity", otherEntity->GetId() );
	args.SetValue( "otherEntityName", otherEntity->GetName() );
	args.SetValue( "otherEntityType", otherEntity->GetType() );

	EventArgs otherArgs;
	otherArgs.SetValue( "otherEntity", entity->GetId() );
	otherArgs.SetValue( "otherEntityName", entity->GetName() );
	otherArgs.SetValue( "otherEntityType", entity->GetType() );

	if ( DoDiscsOverlap( entity->GetPosition(), entity->GetPhysicsRadius(), otherEntity->GetPosition(), otherEntity->GetPhysicsRadius() ) )
	{
		// If entities are already colliding, fire stay
		if ( entity->m_collidingEntities.count( otherEntity->GetId() ) > 0 )
		{
			// Fire events as long as the entities are still valid
			if ( entity != nullptr )
			{
				entity->FireScriptEvent( "OnCollisionStay", &args );
			}
			if ( otherEntity != nullptr )
			{
				otherEntity->FireScriptEvent( "OnCollisionStay", &otherArgs );
			}
		}
		else
		{
			entity->m_collidingEntities.insert( otherEntity->GetId() );
			otherEntity->m_collidingEntities.insert( entity->GetId() );

			// Fire events as long as the entities are still valid
			if ( entity != nullptr )
			{
				entity->FireScriptEvent( "OnCollisionEnter", &args );
			}
			if ( otherEntity != nullptr )
			{
				otherEntity->FireScriptEvent( "OnCollisionEnter", &otherArgs );
			}
		}
	}
	else
	{
		// Entities were colliding earlier, fire leave events
		if ( entity != nullptr && otherEntity != nullptr
			 && entity->m_collidingEntities.count( otherEntity->GetId() ) > 0 )
		{
			entity->m_collidingEntities.erase( otherEntity->GetId() );
			otherEntity->m_collidingEntities.erase( entity->GetId() );

			// Fire events as long as the entities are still valid
			if ( entity != nullptr )
			{
				entity->FireScriptEvent( "OnCollisionLeave", &args );
			}
			if ( otherEntity != nullptr )
			{
				otherEntity->FireScriptEvent( "OnCollisionLeave", &otherArgs );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityVsEntityCollision( Entity& entity1, Entity& entity2 )
{
	// Neither can be moved
	if ( !entity1.m_canBePushed
		 && !entity2.m_canBePushed )
	{
		return;
	}

	// Neither can push
	if ( !entity1.m_canPush
		 && !entity2.m_canPush )
	{
		return;
	}

	float radius1 = entity1.GetPhysicsRadius();
	float radius2 = entity2.GetPhysicsRadius();

	// Both can be moved
	if ( entity1.m_canBePushed
		 && entity2.m_canBePushed )
	{
		if ( entity1.m_canPush
			 && entity2.m_canPush )
		{
			PushDiscsOutOfEachOtherRelativeToMass2D( entity1.m_position, radius1, entity1.GetMass(), entity2.m_position, radius2, entity2.GetMass() );
		}
		
		if ( entity1.m_canPush
			  && !entity2.m_canPush )
		{
			PushDiscOutOfDisc2D( entity2.m_position, radius2, entity1.m_position, radius1 );
		}

		if ( entity2.m_canPush
			  && !entity1.m_canPush )
		{
			PushDiscOutOfDisc2D( entity1.m_position, radius1, entity2.m_position, radius2 );
		}
	}

	// Only entity1 can be moved
	if ( entity1.m_canBePushed
		 && !entity2.m_canBePushed )
	{
		if ( entity2.m_canPush )
		{
			PushDiscOutOfDisc2D( entity1.m_position, radius1, entity2.m_position, radius2 );
		}
	}

	// Only entity2 can be moved
	if ( entity2.m_canBePushed
		 && !entity1.m_canBePushed )
	{
		if ( entity1.m_canPush )
		{
			PushDiscOutOfDisc2D( entity2.m_position, radius2, entity1.m_position, radius1 );
		}
	}
}
