#include "Game/Player.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Player::Player( Game* theGame, const Vec2& position )
	: Entity( theGame, position )
{
	m_entityType = EntityType::ENTITY_TYPE_PLAYER;

	m_physicsRadius = 1.f;
	m_physicsRadiusStack.push_back( m_physicsRadius );

	// Initial vertices
	m_vertexes.push_back( Vertex_PCU( Vec2( -2.f, 0.f ), m_mainColor ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( 2.f, 0.f ), m_mainColor ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( 0.f, 4.f ), m_mainColor ) );
}


//-----------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	if ( m_invincibilityTimer > 0.f )
	{
		m_invincibilityTimer -= deltaSeconds;
	}
	else
	{
		m_isInvincible = false;
	}

	UpdateFromKeyboard( deltaSeconds );
	UpdateFromGamepad( deltaSeconds );

	m_velocity.ClampLength( PLAYER_MAX_SPEED );

	CheckIfOutOfBounds();

	Entity::Update( deltaSeconds );

	// Friction
	if ( m_velocity.GetLength() > .2f )
	{
		m_velocity -= .2f * m_velocity.GetNormalized();
	}

	m_vertexesCopy = m_vertexes;
	Vertex_PCU::TransformVertexArray( m_vertexesCopy, 1.f, m_orientationDegrees, m_position );
}

//-----------------------------------------------------------------------------------------------
void Player::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	const KeyButtonState* keyboardState = g_theInput->GetKeyboardState();

	float moveSpeed = PLAYER_CONTROLLER_SPEED * deltaSeconds;

	if ( keyboardState[KeyCode::KEY_CODE_ID_W].IsPressed()
		 || keyboardState[KeyCode::KEY_CODE_ID_UPARROW].IsPressed() )
	{
		m_orientationDegrees = 90.f;
		m_velocity.y += moveSpeed;
	}
	
	if ( keyboardState[KeyCode::KEY_CODE_ID_A].IsPressed()
		 || keyboardState[KeyCode::KEY_CODE_ID_LEFTARROW].IsPressed() )
	{
		m_orientationDegrees = 180.f;
		m_velocity.x -= moveSpeed;
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_D].IsPressed()
		 || keyboardState[KeyCode::KEY_CODE_ID_RIGHTARROW].IsPressed() )
	{
		m_orientationDegrees = 0.f;
		m_velocity.x += moveSpeed;
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_S].IsPressed()
		 || keyboardState[KeyCode::KEY_CODE_ID_DOWNARROW].IsPressed() )
	{
		m_orientationDegrees = 270.f;
		m_velocity.y -= moveSpeed;
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_SPACEBAR].WasJustPressed() )
	{
		FireProjectile();
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_N].WasJustPressed()
		 && IsDead() )
	{
		Respawn();
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
	const XboxController& controller = g_theInput->GetXboxController( m_controllerID );
	if ( !controller.IsConnected() )
	{
		return;
	}
	
	if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_START ) )
	{
		if ( m_isDead )
		{
			Respawn();
			return;
		}
	}

	if ( m_isDead )
	{
		return;
	}

	// Check if player is thrusting and update acceleration and orientation
	const AnalogJoystick& leftStick = controller.GetLeftJoyStick();
	float leftStickMagnitude = leftStick.GetMagnitude();
	if ( leftStickMagnitude > 0.f )
	{
		m_orientationDegrees = leftStick.GetDegrees();
		m_velocity += leftStickMagnitude * PLAYER_CONTROLLER_SPEED * GetForwardVector();
	}

	if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_A ) )
	{
		FireProjectile();
	}
}

void Player::Respawn()
{
	// Reset member variables and return to starting position
	m_position = Vec2( 0.f, 0.f );
	m_linearAcceleration = Vec2( 0.f, 0.f );
	m_velocity = Vec2( 0.f, 0.f );
	m_angularVelocity = 0.f;
	m_orientationDegrees = 0.f;
	m_isDead = false;
}

void Player::CheckIfOutOfBounds()
{
	if ( m_position.x > WORLD_SIZE_X )
	{
		m_position.x = WORLD_SIZE_X;
	}
	else if ( m_position.x < 0.f )
	{
		m_position.x = 0.f;
	}

	if ( m_position.y > WORLD_SIZE_Y )
	{
		m_position.y = WORLD_SIZE_Y;
	}
	else if ( m_position.y < 0.f )
	{
		m_position.y = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Player::FireProjectile()
{
	// No extra triangles to shoot
	size_t playerVertexesSize = m_vertexes.size();
	if ( playerVertexesSize <= 3 )
	{
		return;
	}

	std::vector<Vertex_PCU> projVertexes;
	
	projVertexes.push_back( m_vertexes[playerVertexesSize - 3] );
	projVertexes.push_back( m_vertexes[playerVertexesSize - 2] );
	projVertexes.push_back( m_vertexes[playerVertexesSize - 1] );

	m_vertexes.pop_back();
	m_vertexes.pop_back();
	m_vertexes.pop_back();

	Entity*& lastEntity = *( m_capturedEntities.end() - 1 );
	if ( lastEntity->GetPhysicsRadius() == *( m_physicsRadiusStack.end() - 1 ) )
	{
		m_physicsRadiusStack.pop_back();
		m_physicsRadius = *( m_physicsRadiusStack.end() - 1 );
	}

	m_game->SpawnEntity( lastEntity, m_position + (lastEntity->GetPhysicsRadius() + m_physicsRadius + 15.f) * GetForwardVector(), PLAYER_SHOOT_SPEED * GetForwardVector() );
	m_capturedEntities.pop_back();
}


//-----------------------------------------------------------------------------------------------
void Player::Render() const
{
	if ( IsDead() )
	{
		return;
	}
		
	g_theRenderer->DrawVertexArray( m_vertexesCopy );
}


//-----------------------------------------------------------------------------------------------
void Player::Die()
{
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
void Player::Collide( const Entity* otherEntity )
{

}


//-----------------------------------------------------------------------------------------------
void Player::DebugRender() const
{
	Entity::DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Player::AddVertexes( std::vector<Vertex_PCU> vertexes )
{
	for ( int vertIndex = 0; vertIndex < (int)vertexes.size(); ++vertIndex)
	{
		m_vertexes.push_back( vertexes[vertIndex] );
	}
}


//-----------------------------------------------------------------------------------------------
void Player::CaptureEntity( Entity* capturedEntity )
{
	m_capturedEntities.push_back( capturedEntity );
	capturedEntity->SetCaptured( true );
	capturedEntity->SetEntityType( EntityType::ENTITY_TYPE_PLAYER_TRIANGLE );

	if ( capturedEntity->GetPhysicsRadius() > m_physicsRadius )
	{
		m_physicsRadiusStack.push_back( capturedEntity->GetPhysicsRadius() );
		m_physicsRadius = capturedEntity->GetPhysicsRadius();
	}
}


//-----------------------------------------------------------------------------------------------
void Player::TakeDamage()
{
	if ( (int)m_capturedEntities.size() == 0 )
	{
		Entity::TakeDamage(1);
	}
	else
	{
		Entity*& lastEntity = *( m_capturedEntities.end() - 1 );
		if ( lastEntity->GetPhysicsRadius() == *( m_physicsRadiusStack.end() - 1 ) )
		{
			m_physicsRadiusStack.pop_back();
			m_physicsRadius = *( m_physicsRadiusStack.end() - 1 );
		}

		m_vertexes.pop_back();
		m_vertexes.pop_back();
		m_vertexes.pop_back();

		lastEntity->TakeDamage( 3 );
		//delete lastEntity;
		//lastEntity = nullptr;

		m_capturedEntities.pop_back();
	}

	m_isInvincible = true;
	m_invincibilityTimer = HIT_FLASH_SECONDS;
	m_hitFlashTimer = HIT_FLASH_SECONDS;
}


//-----------------------------------------------------------------------------------------------
void Player::AddImpulse( float magnitude, const Vec2& direction )
{
	m_velocity += magnitude * direction;
}
