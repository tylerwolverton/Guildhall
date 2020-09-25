#include "Game/Actor.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
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

	m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
	m_rigidbody2D->SetDrag( 5.f );
	m_rigidbody2D->SetLayer( eCollisionLayer::ENEMY );
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
		//UpdateFromGamepad( deltaSeconds );
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
	if ( m_isPlayer )
	{
		m_curHealth = m_entityDef.GetMaxHealth();
		m_rigidbody2D->SetPosition( m_map->GetPlayerStartPos() );
	}
	else
	{
		Entity::Die();
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::SetAsPlayer()
{
	m_controllerID = 0;
	m_isPlayer = true;

	m_faction = eFaction::GOOD;
	m_rigidbody2D->SetLayer( eCollisionLayer::PLAYER );

	DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscCollider( Vec2::ZERO, GetPhysicsRadius() );
	m_rigidbody2D->TakeCollider( discCollider );
}


//-----------------------------------------------------------------------------------------------
void Actor::UpdateFromKeyboard( float deltaSeconds )
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	float impulseMagnitude = m_entityDef.GetWalkSpeed() * deltaSeconds;

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( 0.f, impulseMagnitude ), GetPosition() );
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( -impulseMagnitude, 0.f ), GetPosition() );
	}
	
	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( impulseMagnitude, 0.f ), GetPosition() );
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		m_rigidbody2D->ApplyImpulseAt( Vec2( 0.f, -impulseMagnitude ), GetPosition() );
	}

	bool spawnProj = false;
	Vec2 projPosition = GetPosition();
	float projOrientation = 0.f;
	// Check for attack
	if ( g_inputSystem->WasKeyJustPressed( KEY_UPARROW ) )
	{
		spawnProj = true;
		projPosition += Vec2( 0.f, .6f );
		projOrientation = 90.f;
	}
	else if ( g_inputSystem->WasKeyJustPressed( KEY_RIGHTARROW ) )
	{
		spawnProj = true;
		projPosition += Vec2( .6f, 0.f );
		projOrientation = 0.f;
	}
	else if ( g_inputSystem->WasKeyJustPressed( KEY_LEFTARROW ) )
	{
		spawnProj = true;
		projPosition += Vec2( -.6f, 0.f );
		projOrientation = 180.f;
	}
	else if ( g_inputSystem->WasKeyJustPressed( KEY_DOWNARROW ) )
	{
		spawnProj = true;
		projPosition += Vec2( 0.f, -.6f );
		projOrientation = 270.f;
	}

	if( spawnProj )
	{
		Entity* entity = m_map->SpawnNewEntityOfTypeAtPosition( "Fireball", projPosition );
		entity->Load();
		entity->SetOrientationDegrees( projOrientation );
		entity->SetCollisionLayer( eCollisionLayer::PLAYER_PROJECTILE );
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
