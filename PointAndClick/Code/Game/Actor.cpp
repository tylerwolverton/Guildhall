#include "Game/Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const Vec2& position, ActorDefinition* actorDef )
	: Entity( position, (EntityDefinition*)actorDef )
	, m_actorDef( actorDef )
{
	if( m_actorDef->GetName() == std::string( "Player" ) )
	{
		m_controllerID = 0;
		m_isPlayer = true;
	}

	m_moveTargetLocation = m_position;
}


//-----------------------------------------------------------------------------------------------
Actor::~Actor()
{
}


//-----------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;

	if ( m_isPlayer )
	{
		//UpdateFromKeyboard( deltaSeconds );
		//UpdateFromGamepad( deltaSeconds );
	}
	
	UpdateAnimation();

	Entity::Update( deltaSeconds );

	MoveToTargetLocation();
}


//-----------------------------------------------------------------------------------------------
void Actor::Render() const
{
	if ( m_curAnimDef == nullptr )
	{
		return;
	}

	const SpriteDefinition& spriteDef = m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime );
		
	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_actorDef->m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, m_position );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Actor::Die()
{
	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
void Actor::ExecuteAction( const std::string& nounName )
{
	m_isExecutingAction = true;
	m_targetNoun = nounName;
}


//-----------------------------------------------------------------------------------------------
void Actor::StopExecutingAction()
{
	m_isExecutingAction = false;
	m_targetNoun = "";
	m_curVerbState = eVerbState::NONE;
	g_game->ClearCurrentActionText();
}


//-----------------------------------------------------------------------------------------------
void Actor::UpdateAnimation()
{
	if ( m_velocity.x > 0.05f )
	{
		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveEast" );
	}
	else if ( m_velocity.x < -0.05f )
	{
		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveWest" );
	}
	else if ( m_velocity.y > 0.05f )
	{
		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveNorth" );
	}
	else if ( m_velocity.y < -0.05f )
	{
		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveSouth" );
	}
	else
	{
		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "Idle" );
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::MoveToTargetLocation()
{
	float distanceToLocation =  m_moveTargetLocation.x - m_position.x;

	if ( m_isExecutingAction
		 && m_map != nullptr 
		 && m_curVerbState != eVerbState::NONE )
	{
		Entity* targetEntity = m_map->GetEntityByName( m_targetNoun );
		if ( targetEntity != nullptr )
		{
			if ( DoDiscsOverlap( m_position, m_actorDef->m_physicsRadius, targetEntity->GetPosition(), targetEntity->GetPhysicsRadius() ) )
			{
				EventArgs args;
				args.SetValue( "Type", (int)m_curVerbState );
				args.SetValue( "Position", targetEntity->GetPosition() );
				g_eventSystem->FireEvent( "VerbAction", &args );

				m_curVerbState = eVerbState::NONE;
				g_game->ClearCurrentActionText();

				m_moveTargetLocation = m_position;
				m_targetNoun = "";
				m_isExecutingAction = false;
			}
		}
	}

	if ( fabsf(distanceToLocation) > 0.1f )
	{
		m_velocity.x = distanceToLocation / fabsf( distanceToLocation ) * m_actorDef->m_walkSpeed;
		m_velocity.y = 0.f;	
	}
}
