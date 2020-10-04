#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Scripting/ZephyrScript.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( const EntityDefinition& entityDef, Map* map )
	: m_entityDef( entityDef )
	, m_map( map )
{
	m_curHealth = m_entityDef.GetMaxHealth();

	m_rigidbody2D = g_physicsSystem2D->CreateRigidbody();
	
	m_rigidbody2D->SetLayer( m_entityDef.GetCollisionLayer() );
	if( m_entityDef.GetCollisionLayer() == eCollisionLayer::STATIC_ENVIRONMENT )
	{
		m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_STATIC );
	}

	m_rigidbody2D->m_userProperties.SetValue( "entity", (void*)this );

	RegisterUserEvents();

	ZephyrScriptDefinition* scriptDef = entityDef.GetZephyrScriptDefinition();
	if ( scriptDef != nullptr )
	{
		m_scriptObj = new ZephyrScript( *scriptDef, this );
	}
}


//-----------------------------------------------------------------------------------------------
Entity::~Entity()
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->Destroy();
		m_rigidbody2D = nullptr;
	}

	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_scriptObj );

	PTR_VECTOR_SAFE_DELETE( m_inventory );
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;

	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->Update();
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	SpriteAnimationSetDefinition* walkAnimSetDef = m_entityDef.GetSpriteAnimSetDef( "Walk" );
	SpriteAnimDefinition* walkAnimDef = nullptr;
	if ( walkAnimSetDef == nullptr 
		 || m_rigidbody2D == nullptr )
	{
		return;
	}

	walkAnimDef = walkAnimSetDef->GetSpriteAnimationDefForDirection( m_rigidbody2D->GetVelocity() );
	
	const SpriteDefinition& spriteDef = walkAnimDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_entityDef.m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, GetPosition() );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	if ( IsDead() )
	{
		return;
	}

	m_isDead = true;

	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->FireDieEvent();
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, GetPosition(), m_entityDef.m_physicsRadius, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	DrawAABB2Outline( g_renderer, GetPosition(), m_entityDef.m_localDrawBounds, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetPosition() const
{
	if ( m_rigidbody2D != nullptr )
	{
		return m_rigidbody2D->GetPosition();
	}

	return Vec2::ZERO;
}


//-----------------------------------------------------------------------------------------------
void Entity::SetPosition( const Vec2& position )
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->SetPosition( position );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::SetCollisionLayer( uint layer )
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->SetLayer( layer );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::AddItemToInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] == nullptr )
		{
			m_inventory[itemIdx] = item;
			return;
		}
	}

	m_inventory.push_back( item );
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( const std::string& itemId )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			&& m_inventory[itemIdx]->GetId() == itemId )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			&& m_inventory[itemIdx] == item )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Entity::FireSpawnEvent()
{
	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->FireSpawnEvent();
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damage )
{
	if ( IsDead() )
	{
		return;
	}

	m_curHealth -= damage;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	g_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void Entity::Load()
{
	m_rigidbody2D->Enable();

	DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscCollider( Vec2::ZERO, GetPhysicsRadius() );
	m_rigidbody2D->TakeCollider( discCollider );
}


//-----------------------------------------------------------------------------------------------
void Entity::Unload()
{
	m_rigidbody2D->Disable();

	g_physicsSystem2D->DestroyCollider( m_rigidbody2D->GetCollider() );
}


//-----------------------------------------------------------------------------------------------
void Entity::ReloadZephyrScript()
{
	if ( m_scriptObj != nullptr )
	{
		PTR_SAFE_DELETE( m_scriptObj );

		ZephyrScriptDefinition* scriptDef = m_entityDef.GetZephyrScriptDefinition();
		if ( scriptDef != nullptr )
		{
			m_scriptObj = new ZephyrScript( *scriptDef, this );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RegisterUserEvents()
{
	std::map<std::string, std::string>const& registeredEvents = m_entityDef.GetRegisteredEvents();

	for ( auto it = registeredEvents.begin(); it != registeredEvents.end(); ++it )
	{ 
		g_eventSystem->RegisterMethodEvent( it->first, "", GAME, this, &Entity::FireCorrespondingEvent );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::FireCorrespondingEvent( EventArgs* args )
{
	std::string eventName = args->GetValue( "eventName", "" );
	if ( eventName.empty() )
	{
		return;
	}

	std::map<std::string, std::string> registeredEvents = m_entityDef.GetRegisteredEvents();

	auto iter = registeredEvents.find( eventName );
	if ( iter == registeredEvents.end() )
	{
		g_devConsole->PrintWarning( Stringf( "Event: '%s' had no corresponding event to fire", eventName.c_str() ) );
		return;
	}

	g_devConsole->PrintString( Stringf( "Firing event '%s' from '%s'", iter->second.c_str(), m_entityDef.GetName().c_str() ) );
	g_eventSystem->FireEvent( iter->second );
}


//-----------------------------------------------------------------------------------------------
//void Entity::ApplyFriction()
//{
//	if ( m_velocity.GetLength() > PHYSICS_FRICTION_FRACTION )
//	{
//		m_velocity -= m_velocity * PHYSICS_FRICTION_FRACTION;
//	}
//	else
//	{
//		m_velocity = Vec2( 0.f, 0.f );
//	}
//}
