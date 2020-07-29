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
#include "Game/Map.hpp"
#include "Game/EntityDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const Vec2& position, EntityDefinition* entityDef )
	: Entity( position, entityDef )
{
	if( entityDef->GetName() == std::string( "Player" ) )
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
	Entity::Update( deltaSeconds );

	MoveToTargetLocation();
}


//-----------------------------------------------------------------------------------------------
void Actor::Render() const
{
	Entity::Render();
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
			if ( DoDiscsOverlap( m_position, GetPhysicsRadius(), targetEntity->GetPosition(), targetEntity->GetPhysicsRadius() ) )
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
		m_velocity.x = distanceToLocation / fabsf( distanceToLocation ) * GetWalkSpeed();
		m_velocity.y = 0.f;	
	}
}
