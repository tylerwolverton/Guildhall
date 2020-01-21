#include "Player.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Player::Player( EntityFaction faction, const Vec2& position )
	: Entity( ENTITY_TYPE_PLAYER, faction, position )
{
	m_physicsRadius = PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_COSMETIC_RADIUS;
	m_maxTurnSpeed = PLAYER_MAX_TURN_SPEED;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
Player::~Player()
{
}


//-----------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );
	UpdateFromGamepad( deltaSeconds );

	Entity::Update( deltaSeconds );

	m_velocity.ClampLength( PLAYER_MAX_SPEED );
}


//-----------------------------------------------------------------------------------------------
void Player::Render() const
{
	std::vector<Vertex_PCU> vertexesCopy( m_vertexes );
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, m_orientationDegrees, m_position );

	g_renderer->BindTexture( m_tankBodyTexture );
	g_renderer->DrawVertexArray( vertexesCopy );
}


//-----------------------------------------------------------------------------------------------
void Player::Die()
{

}


//-----------------------------------------------------------------------------------------------
void Player::PopulateVertexes()
{
	m_tankBodyTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/PlayerTankBase.png" );

	Rgba8 white( 255, 255, 255 );

	m_vertexes.push_back( Vertex_PCU( Vec2( -.4f, -.4f ),	white, Vec2( 0.f, 0.f ) ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( .4f, -.4f ),	white, Vec2( 1.f, 0.f ) ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( .4f, .4f ),		white, Vec2( 1.f, 1.f ) ) );

	m_vertexes.push_back( Vertex_PCU( Vec2( -.4f, -.4f ),	white, Vec2( 0.f, 0.f ) ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( .4f, .4f ),		white, Vec2( 1.f, 1.f ) ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( -.4f, .4f ),	white, Vec2( 0.f, 1.f ) ) );
}


//-----------------------------------------------------------------------------------------------
void Player::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
		
	if ( g_inputSystem->IsKeyPressed( 'W' )
		 || g_inputSystem->IsKeyPressed( KEY_UPARROW ) )
	{
		m_velocity.y += PLAYER_SPEED;
		TurnToward( 90.f, deltaSeconds );
	}

	if ( g_inputSystem->IsKeyPressed( 'A' )
		 || g_inputSystem->IsKeyPressed( KEY_LEFTARROW ) )
	{
		m_velocity.x -= PLAYER_SPEED;
		TurnToward( 180.f, deltaSeconds );

	}

	if ( g_inputSystem->IsKeyPressed( 'D' )
		 || g_inputSystem->IsKeyPressed( KEY_RIGHTARROW ) )
	{
		m_velocity.x += PLAYER_SPEED;
		TurnToward( 0.f, deltaSeconds );
	}

	if ( g_inputSystem->IsKeyPressed( 'S' )
		 || g_inputSystem->IsKeyPressed( KEY_DOWNARROW ) )
	{
		m_velocity.y -= PLAYER_SPEED;
		TurnToward( 270.f, deltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void Player::UpdateFromGamepad( float deltaSeconds )
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
		TurnToward( leftStick.GetDegrees(), deltaSeconds );
		m_velocity += leftStickMagnitude * PLAYER_SPEED * GetForwardVector();
	}
}
