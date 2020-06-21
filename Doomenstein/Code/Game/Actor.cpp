#include "Game/Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const EntityDefinition& entityDef )
	: Entity( entityDef )
{
	/*if( m_actorDef->GetName() == std::string( "Player" ) )
	{
		m_controllerID = 0;
		m_isPlayer = true;
	}*/
}


//-----------------------------------------------------------------------------------------------
Actor::~Actor()
{
}


//-----------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	//m_cumulativeTime += deltaSeconds;

	/*if ( m_isPlayer )
	{
		UpdateFromKeyboard( deltaSeconds );
		UpdateFromGamepad( deltaSeconds );
	}*/
	
	//UpdateAnimation();

	Entity::Update( deltaSeconds );

	//m_velocity.ClampLength( PLAYER_MAX_SPEED );
}


//-----------------------------------------------------------------------------------------------
void Actor::Render() const
{
	/*const SpriteDefinition& spriteDef = m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime );
		
	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertices;
	AppendVertsForAABB2D( vertices, m_actorDef->m_localDrawBounds, Rgba8::WHITE,
									  mins,
									  maxs );

	Vertex_PCU::TransformVertexArray( vertices, 1.f, 0.f, m_position );

	g_renderer->BindDiffuseTexture( &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertices );*/
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

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		m_velocity.y += m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		m_velocity.x -= m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		m_velocity.x += m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		m_velocity.y -= m_entityDef.GetWalkSpeed();
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
		m_velocity += leftStickMagnitude * m_entityDef.GetWalkSpeed() * GetForwardVector();
	}
}

