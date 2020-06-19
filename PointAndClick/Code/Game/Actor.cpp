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
		UpdateFromKeyboard( deltaSeconds );
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
void Actor::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	
	if ( g_inputSystem->WasKeyJustPressed( MOUSE_RBUTTON ) )
	{
		m_moveTargetLocation = g_game->GetMouseWorldPosition();

	}
	if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
	{
		EventArgs args;
		//args.SetValue( "Type", "PickUp" );
		args.SetValue( "Position", g_game->GetMouseWorldPosition() );
		g_eventSystem->FireEvent( "VerbAction", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::UpdateFromGamepad( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	// No controller assigned
	if ( m_controllerID < 0 )
	{
		return;
	}

	// If controller isn't connected return early
	const XboxController& controller = g_inputSystem->GetXboxController( m_controllerID );
	if ( !controller.IsConnected() )
	{
		return;
	}

	if ( m_isDead )
	{
		return;
	}

	const AnalogJoystick& leftStick = controller.GetLeftJoyStick();
	float leftStickMagnitude = leftStick.GetMagnitude();
	if ( leftStickMagnitude > 0.f )
	{
		m_orientationDegrees = leftStick.GetDegrees();
		m_velocity += leftStickMagnitude * m_actorDef->m_walkSpeed * GetForwardVector();
	}
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
	//float distanceToLocation = GetDistance2D( m_position, m_moveTargetLocation );

	if ( fabsf(distanceToLocation) > 0.1f )
	{
		m_velocity.x = distanceToLocation / fabsf( distanceToLocation ) * m_actorDef->m_walkSpeed;
		//m_velocity = GetNormalizedDirectionFromAToB( m_position, m_moveTargetLocation ) * m_actorDef->m_walkSpeed;
		m_velocity.y = 0.f;
	}
}
