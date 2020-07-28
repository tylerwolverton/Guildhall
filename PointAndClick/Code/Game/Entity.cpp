#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Time.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( const Vec2& position, EntityDefinition* entityDef )
	: m_position( position )
	, m_entityDef( entityDef )
{
	PopulateVertexes();

	m_curAnimDef = m_entityDef->GetSpriteAnimDef( "Idle" );
}


//-----------------------------------------------------------------------------------------------
Entity::~Entity()
{
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
	if ( m_isInPlayerInventory )
	{
		return;
	}

	UpdateAnimation();

	// vel += acceleration * dt;
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
	// pos += vel * dt;
	m_position += m_velocity * deltaSeconds;

	//update orientation
	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	ApplyFriction();
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	if ( m_isInPlayerInventory )
	{
		return;
	}

	if ( m_curAnimDef != nullptr )
	{
		const SpriteDefinition& spriteDef = m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime );

		Vec2 mins, maxs;
		spriteDef.GetUVs( mins, maxs );

		std::vector<Vertex_PCU> vertexes;
		AppendVertsForAABB2D( vertexes, m_entityDef->m_localDrawBounds, Rgba8::WHITE, mins, maxs );

		Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, m_position );

		g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
		g_renderer->DrawVertexArray( vertexes );
	}
	else if ( m_texture != nullptr )
	{
		std::vector<Vertex_PCU> vertexes;
		AppendVertsForAABB2D( vertexes, m_entityDef->m_localDrawBounds, Rgba8::WHITE );

		Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, m_position );

		g_renderer->BindTexture( 0, m_texture );
		g_renderer->DrawVertexArray( vertexes );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, m_position, m_entityDef->m_physicsRadius, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	DrawAABB2Outline( g_renderer, m_position, m_entityDef->m_localDrawBounds, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damage )
{
	m_curHealth -= damage;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	g_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void Entity::ApplyFriction()
{
	if ( m_velocity.GetLength() > PHYSICS_FRICTION_FRACTION )
	{
		m_velocity -= m_velocity * PHYSICS_FRICTION_FRACTION;
	}
	else
	{
		m_velocity = Vec2( 0.f, 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
SpriteDefinition* Entity::GetSpriteDef() const
{
	if ( m_curAnimDef == nullptr )
	{
		return nullptr;
	}

	return const_cast<SpriteDefinition*>( &( m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime ) ) );
}


//-----------------------------------------------------------------------------------------------
void Entity::AddVerbState( eVerbState verbState, NamedProperties* properties )
{
	auto mapIter = m_entityDef->m_verbPropertiesMap.find( verbState );
	if ( mapIter != m_entityDef->m_verbPropertiesMap.end() )
	{
		m_entityDef->m_verbPropertiesMap.erase( verbState );
	}

	m_entityDef->m_verbPropertiesMap[verbState] = properties;
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveVerbState( eVerbState verbState )
{
	m_entityDef->m_verbPropertiesMap.erase( verbState );
}


//-----------------------------------------------------------------------------------------------
void Entity::HandleVerbAction( eVerbState verbState )
{
	NamedProperties* verbEventProperties = m_entityDef->GetVerbEventProperties( verbState );

	if ( verbEventProperties == nullptr )
	{
		g_game->PrintTextOverPlayer( "Hmmm, that's not going to work" );
		
		g_game->ClearCurrentActionText();

		return;
	}

	EventArgs args;
	args.SetValue( "target", (void*)this );
	args.SetValue( "properties", (void*)verbEventProperties );

	std::string verbEventName = verbEventProperties->GetValue( "eventName", "" );
	if ( verbEventName == "" )
	{
		g_devConsole->PrintError( Stringf( "Invalid event name '%s' seen for verb state '%s'", verbEventName.c_str(), GetEventNameForVerbState( verbState ).c_str() ) );
		return;
	}

	g_eventSystem->FireEvent( verbEventName, &args );
}


//-----------------------------------------------------------------------------------------------
void Entity::PopulateVertexes()
{	
	AppendVertsForAABB2D( m_vertexes, m_entityDef->m_localDrawBounds, Rgba8::WHITE,
									  m_entityDef->m_uvCoords.mins,
									  m_entityDef->m_uvCoords.maxs );
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdateAnimation()
{
	if ( m_velocity.x > 0.05f )
	{
		m_curAnimDef = m_entityDef->GetSpriteAnimDef( "MoveEast" );
	}
	else if ( m_velocity.x < -0.05f )
	{
		m_curAnimDef = m_entityDef->GetSpriteAnimDef( "MoveWest" );
	}
	else if ( m_velocity.y > 0.05f )
	{
		m_curAnimDef = m_entityDef->GetSpriteAnimDef( "MoveNorth" );
	}
	else if ( m_velocity.y < -0.05f )
	{
		m_curAnimDef = m_entityDef->GetSpriteAnimDef( "MoveSouth" );
	}
	else
	{
		m_curAnimDef = m_entityDef->GetSpriteAnimDef( "Idle" );
	}
}
