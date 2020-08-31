#include "Game/Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_faction = eFaction::EVIL;

	m_canBePushedByWalls = true;
	m_canBePushedByEntities = true;
	m_canPushEntities = true;
}


//-----------------------------------------------------------------------------------------------
Actor::~Actor()
{
}


//-----------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	if ( m_isPlayer )
	{
		UpdateFromKeyboard( deltaSeconds );
		UpdateFromGamepad( deltaSeconds );
	}

	//UpdateAnimation();
	//SetOrientationDegrees( m_velocity.GetOrientationDegrees() );

	Entity::Update( deltaSeconds );
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
void Actor::SetAsPlayer()
{
	m_controllerID = 0;
	m_isPlayer = true;

	m_faction = eFaction::GOOD;
}


//-----------------------------------------------------------------------------------------------
void Actor::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_inputSystem->IsKeyPressed( 'W' )
		 || g_inputSystem->IsKeyPressed( KEY_UPARROW ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( 0.f, 100.f * m_entityDef.GetWalkSpeed() * deltaSeconds ), GetPosition() );
		//m_rigidbody2D->Translate2D( Vec2( 0.f, m_entityDef.GetWalkSpeed() * deltaSeconds ) );
		//m_velocity.y += m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->IsKeyPressed( 'A' )
		 || g_inputSystem->IsKeyPressed( KEY_LEFTARROW ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( -100.f * m_entityDef.GetWalkSpeed() * deltaSeconds, 0.f ), GetPosition() );
		//m_rigidbody2D->ApplyImpulseAt( Vec2( -m_entityDef.GetWalkSpeed(), 0.f ), m_position );
		//m_velocity.x -= m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->IsKeyPressed( 'D' )
		 || g_inputSystem->IsKeyPressed( KEY_RIGHTARROW ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( 100.f * m_entityDef.GetWalkSpeed() * deltaSeconds, 0.f ), GetPosition() );
		//m_rigidbody2D->ApplyImpulseAt( Vec2( m_entityDef.GetWalkSpeed(), 0.f ), m_position );
		//m_velocity.x += m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->IsKeyPressed( 'S' )
		 || g_inputSystem->IsKeyPressed( KEY_DOWNARROW ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( 0.f, -100.f * m_entityDef.GetWalkSpeed() * deltaSeconds ), GetPosition() );
		//m_rigidbody2D->ApplyImpulseAt( Vec2( 0.f, -m_entityDef.GetWalkSpeed() ), m_position );
		//m_velocity.y -= m_entityDef.GetWalkSpeed();
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_SPACEBAR ) )
	{
		Entity* entity = m_map->SpawnNewEntityOfTypeAtPosition( "Fireball", GetPosition() );
		entity->SetOrientationDegrees( GetForwardVector().GetOrientationDegrees() );
		entity->SetFaction( eFaction::GOOD );
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
		//m_velocity += leftStickMagnitude * m_entityDef.GetWalkSpeed() * GetForwardVector();
	}
}


//-----------------------------------------------------------------------------------------------
//void Actor::UpdateAnimation()
//{
//	if ( m_velocity.x > 0.05f )
//	{
//		m_curAnimDef = m_entityDef->GetSpriteAnimDef( "MoveEast" );
//	}
//	else if ( m_velocity.x < -0.05f )
//	{
//		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveWest" );
//	}
//	else if ( m_velocity.y > 0.05f )
//	{
//		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveNorth" );
//	}
//	else if ( m_velocity.y < -0.05f )
//	{
//		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "MoveSouth" );
//	}
//	else
//	{
//		m_curAnimDef = m_actorDef->GetSpriteAnimDef( "Idle" );
//	}
//}
