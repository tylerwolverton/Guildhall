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
#include "Game/Scripting/ZephyrScript.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_faction = eFaction::EVIL;
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

	// Draw health bar if damaged
	if ( m_curHealth == m_entityDef.GetMaxHealth() )
	{
		return;
	}

	std::vector<Vertex_PCU> hpVertices;
	Vec2 hpBarMins( m_entityDef.m_localDrawBounds.mins );
	Vec2 hpBarMaxs( m_entityDef.m_localDrawBounds.maxs.x, m_entityDef.m_localDrawBounds.mins.y + .065f );

	AABB2 hpBarBackground( hpBarMins, hpBarMaxs );

	AppendVertsForAABB2D( hpVertices, hpBarBackground, Rgba8::BLACK );

	AABB2 hpRemaining( hpBarBackground );
	float hpFraction = (float)m_curHealth / (float)m_entityDef.GetMaxHealth();
	hpRemaining.ChopOffRight( 1.f - hpFraction, 0.f );

	Rgba8 hpColor = Rgba8::GREEN;
	if ( hpFraction < .25f )
	{
		hpColor = Rgba8::RED;
	}
	else if ( hpFraction < .5f )
	{
		hpColor = Rgba8::YELLOW;
	}

	AppendVertsForAABB2D( hpVertices, hpRemaining, hpColor );

	Vec2 hpPosition( GetPosition() );
	hpPosition.y += m_entityDef.m_localDrawBounds.GetHeight() + .001f;

	Vertex_PCU::TransformVertexArray( hpVertices, 1.f, 0.f, hpPosition );

	g_renderer->BindDiffuseTexture( nullptr );
	g_renderer->DrawVertexArray( hpVertices );
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

	m_name = "player";
	m_faction = eFaction::GOOD;
	m_rigidbody2D->SetLayer( eCollisionLayer::PLAYER );

	DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscCollider( Vec2::ZERO, GetPhysicsRadius() );
	m_rigidbody2D->TakeCollider( discCollider );
}


//-----------------------------------------------------------------------------------------------
void Actor::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_devConsole->IsOpen() )
	{
		return;
	}
	
	switch ( g_game->GetGameState() )
	{
		case eGameState::PLAYING:
		{
			for ( auto& registeredKey : m_registeredKeyEvents )
			{
				if ( g_inputSystem->IsKeyPressed( registeredKey.first ) )
				{
					for ( auto& eventName : registeredKey.second )
					{
						EventArgs args;
						FireScriptEvent( eventName, &args );
					}
				}
			}

			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ENTER )
				 || g_inputSystem->ConsumeAllKeyPresses( KEY_SPACEBAR ) )
			{
				Vec2 testPoint = GetPosition() + m_forwardVector * ( GetPhysicsRadius() + .1f );
				Entity* targetEntity = m_map->GetEntityAtPosition( testPoint );
				if ( targetEntity != nullptr )
				{
					EventArgs args;
					targetEntity->FireScriptEvent( "PlayerInteracted", &args );
				}
			}
		}
		break;

		case eGameState::DIALOGUE:
		{
			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ENTER )
				 || g_inputSystem->ConsumeAllKeyPresses( KEY_SPACEBAR ) )
			{
				Vec2 testPoint = GetPosition() + m_forwardVector * ( GetPhysicsRadius() + .15f );
				Entity* targetEntity = m_map->GetEntityAtPosition( testPoint );
				if ( targetEntity != nullptr )
				{
					g_game->SelectInDialogue( targetEntity );
				}
				else
				{
					// Return to playing state to avoid being trapped in dialogue
					g_devConsole->PrintWarning( "Game was in dialogue state but player had no dialogue partner" );
					g_game->ChangeGameState( eGameState::PLAYING );
				}
			}
		}
		break;
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
