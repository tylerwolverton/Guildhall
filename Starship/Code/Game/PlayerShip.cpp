#include "PlayerShip.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
PlayerShip::PlayerShip(Game* theGame, const Vec2& position)
	: Entity(theGame, position)
{
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_mainColor = Rgba8(75, 150, 250);
	m_numDebrisPieces = NUM_PLAYER_SHIP_DEBRIS_PIECES;
	m_health = 1;
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::Update( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );
	UpdateFromGamepad( deltaSeconds );

	// Set vibration for controller if requested
	if ( m_vibrationSeconds > 0.f )
	{
		m_vibrationSeconds -= deltaSeconds;
	}
	else
	{
		m_leftVibrationFraction = 0.f;
		m_rightVibrationFraction = 0.f;
	}
	g_inputSystem->SetXboxControllerVibrationLevels( m_controllerID, m_leftVibrationFraction, m_rightVibrationFraction );

	// Handle starburst cooldown
	if ( m_starburstCooldownSeconds > 0.f )
	{
		m_starburstCooldownSeconds -= deltaSeconds;
		m_starburstCooldownAlpha = (unsigned char)40;
	}
	else
	{
		m_canFireStarburstBullet = true;
		m_starburstCooldownAlpha = (unsigned char)255;

	}

	if ( m_isDead )
	{
		return;
	}

	TurnAndThrust( deltaSeconds );
	CheckForWallBounce();

	// Calculate and clamp velocity
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
	m_velocity.ClampLength( PLAYER_SHIP_MAX_SPEED );

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::TurnAndThrust( float deltaSeconds )
{
	if ( IsDead()
		|| m_game->GetGameState() == GameState::GAME_STATE_ATTRACT )
	{
		return;
	}

	if ( m_isShipAccelerating 
		 || m_isShipAcceleratingFromJoystick )
	{
		m_linearAcceleration = GetForwardVector() * PLAYER_SHIP_ACCELERATION * m_thrustFraction;
	}

	if ( m_isShipTurningLeft )
	{
		m_orientationDegrees += PLAYER_SHIP_TURN_SPEED * deltaSeconds;
	}

	if ( m_isShipTurningRight )
	{
		m_orientationDegrees += -PLAYER_SHIP_TURN_SPEED * deltaSeconds;
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::CheckForWallBounce()
{
	if ( IsDead() )
	{
		return;
	}

	// Snap back into level and flip the velocity component
	
	// Colliding with sides
	if ( m_position.x + m_cosmeticRadius >= WINDOW_WIDTH )
	{
		m_position.x = WINDOW_WIDTH - m_cosmeticRadius;
		m_velocity.x *= -1.f;
	}
	else if( m_position.x - m_cosmeticRadius <= 0 )
	{
		m_position.x = m_cosmeticRadius;
		m_velocity.x *= -1.f;
	}

	// Colliding with top or bottom
	if ( m_position.y + m_cosmeticRadius >= WINDOW_HEIGHT )
	{
		m_position.y = WINDOW_HEIGHT - m_cosmeticRadius;
		m_velocity.y *= -1.f;
	}
	else if( m_position.y - m_cosmeticRadius <= 0)
	{
		m_position.y = m_cosmeticRadius;
		m_velocity.y *= -1.f;
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::HandleBulletFired()
{
	if ( IsDead()
		 || !m_canFireBullet 
		 || m_game->GetGameState() == GameState::GAME_STATE_ATTRACT )
	{
		return;
	}

	Vec2 bulletSpawnPos = m_position;
	Vec2 playerOrientationNormalized = GetForwardVector().GetNormalized();
	// Scale by cosmetic radius so bullet spawns with it's tail on the nose of the ship
	bulletSpawnPos += playerOrientationNormalized * BULLET_COSMETIC_RADIUS; 
	
	m_game->SpawnBullet( bulletSpawnPos, playerOrientationNormalized );

	m_canFireBullet = false;

	m_leftVibrationFraction = .03f;
	m_rightVibrationFraction = .03f;

	m_vibrationSeconds = .1f;
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::HandleStarburstBulletFired()
{
	if ( IsDead()
		|| !m_canFireStarburstBullet
		|| m_game->GetGameState() == GameState::GAME_STATE_ATTRACT )
	{
		return;
	}

	Vec2 bulletSpawnPos = m_position;
	Vec2 playerOrientationNormalized = GetForwardVector().GetNormalized();
	// Scale by cosmetic radius so bullet spawns with it's tail on the nose of the ship
	bulletSpawnPos += playerOrientationNormalized * BULLET_COSMETIC_RADIUS;

	m_game->SpawnStarburstBullet(bulletSpawnPos, playerOrientationNormalized);

	m_canFireStarburstBullet = false;

	m_leftVibrationFraction = .03f;
	m_rightVibrationFraction = .03f;

	m_vibrationSeconds = .1f;

	m_starburstCooldownSeconds = STARBURST_COOLDOWN;
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Render() const
{
	if ( IsDead() )
	{
		return;
	}

	// Move to update if pause blocks update
	//float thrustTailPos = -2.f + ( thrustRandomness * m_thrustFraction * -4.f );

	Vertex_PCU playerShipVertexes[] =
	{
		// Left wing
		Vertex_PCU( Vec2( -2.f, 1.f ),	m_mainColor ),
		Vertex_PCU( Vec2( 2.f, 1.f ),	m_mainColor),
		Vertex_PCU( Vec2( 0.f, 2.f ),	m_mainColor),

		// Top left of body
		Vertex_PCU( Vec2( -2.f, -1.f ), m_mainColor),
		Vertex_PCU( Vec2( 0.f, 1.f ),	m_mainColor),
		Vertex_PCU( Vec2( -2.f, 1.f ),	m_mainColor),

		// Bottom right of body
		Vertex_PCU( Vec2( -2.f, -1.f ), m_mainColor),
		Vertex_PCU( Vec2( 0.f, -1.f ),	m_mainColor),
		Vertex_PCU( Vec2( 0.f, 1.f ),	m_mainColor),

		// Nose
		Vertex_PCU( Vec2( 0.f, -1.f ),	m_mainColor),
		Vertex_PCU( Vec2( 1.f, 0.f ),	m_mainColor),
		Vertex_PCU( Vec2( 0.f, 1.f ),	m_mainColor),

		// Right wing
		Vertex_PCU( Vec2( -2.f, -1.f ), m_mainColor),
		Vertex_PCU( Vec2( 0.f, -2.f ),	m_mainColor),
		Vertex_PCU( Vec2( 2.f, -1.f ),	m_mainColor)
	};

	constexpr int NUM_VERTEXES = sizeof( playerShipVertexes ) / sizeof( playerShipVertexes[0] );
	Vertex_PCU::TransformVertexArray( playerShipVertexes, NUM_VERTEXES, 1.f, m_orientationDegrees, m_position );

	g_renderer->DrawVertexArray( NUM_VERTEXES, playerShipVertexes );
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::RenderRemainingLives() const
{
	Rgba8 iconColor = m_mainColor;
	iconColor.a = (unsigned char)150;

	Vertex_PCU playerShipVertexes[] =
	{
		// Left wing
		Vertex_PCU( Vec2( -2.f, 1.f ),	iconColor ),
		Vertex_PCU( Vec2( 2.f, 1.f ),	iconColor ),
		Vertex_PCU( Vec2( 0.f, 2.f ),	iconColor ),
												    
		// Top left of body						    
		Vertex_PCU( Vec2( -2.f, -1.f ), iconColor ),
		Vertex_PCU( Vec2( 0.f, 1.f ),	iconColor ),
		Vertex_PCU( Vec2( -2.f, 1.f ),	iconColor ),
												     
		// Bottom right of body					     
		Vertex_PCU( Vec2( -2.f, -1.f ), iconColor ),
		Vertex_PCU( Vec2( 0.f, -1.f ),	iconColor ),
		Vertex_PCU( Vec2( 0.f, 1.f ),	iconColor ),
												     
		// Nose									     
		Vertex_PCU( Vec2( 0.f, -1.f ),	iconColor ),
		Vertex_PCU( Vec2( 1.f, 0.f ),	iconColor ),
		Vertex_PCU( Vec2( 0.f, 1.f ),	iconColor ),
												     
		// Right wing							     
		Vertex_PCU( Vec2( -2.f, -1.f ), iconColor ),
		Vertex_PCU( Vec2( 0.f, -2.f ),	iconColor ),
		Vertex_PCU( Vec2( 2.f, -1.f ),	iconColor )
	};

	constexpr int NUM_VERTEXES = sizeof( playerShipVertexes ) / sizeof( playerShipVertexes[0] );

	// Make a copy of the vertices for each extra life icon
	for ( int lifeIndex = 1; lifeIndex < m_remainingLives; ++lifeIndex )
	{
		Vertex_PCU copyOfLocalVerts[NUM_VERTEXES];
		for ( int vertexIndex = 0; vertexIndex < NUM_VERTEXES; ++vertexIndex )
		{
			copyOfLocalVerts[vertexIndex] = playerShipVertexes[vertexIndex];
		}

		Vec2 position( 4.f * (float)lifeIndex, WINDOW_HEIGHT - 3.f );
		Vertex_PCU::TransformVertexArray( copyOfLocalVerts, NUM_VERTEXES, .7f, 90.f, position );

		g_renderer->DrawVertexArray( NUM_VERTEXES, copyOfLocalVerts );
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::RenderCooldowns() const
{
	Rgba8 noseColor(255, 255, 0, m_starburstCooldownAlpha);
	Rgba8 sideColor(255, 100, 0, m_starburstCooldownAlpha);
	Rgba8 tailColor(255, 100, 0, 0);

	Vertex_PCU starburstCooldownVertices[] =
	{
		// Nose
		Vertex_PCU(Vec2(0.f, 0.5f), sideColor),
		Vertex_PCU(Vec2(0.5f, 0.f), noseColor),
		Vertex_PCU(Vec2(0.f, -0.5f), sideColor),

		// Tail
		Vertex_PCU(Vec2(-2.f, 0.f), tailColor),
		Vertex_PCU(Vec2(0.f, -0.5f), sideColor),
		Vertex_PCU(Vec2(0.f, 0.5f), sideColor)
	};

	constexpr int NUM_VERTEXES = sizeof(starburstCooldownVertices) / sizeof(starburstCooldownVertices[0]);

	Vertex_PCU::TransformVertexArray(starburstCooldownVertices, NUM_VERTEXES, 1.5f, 0.f, Vec2( 5.f, WINDOW_HEIGHT - 8.f ) );

	g_renderer->DrawVertexArray(NUM_VERTEXES, starburstCooldownVertices);
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Die()
{
	m_game->AddScreenShakeIntensity( 1.f );
	m_leftVibrationFraction += 1.f;
	m_rightVibrationFraction += 1.f;
	m_vibrationSeconds += .5f;

	Entity::Die();

	m_remainingLives--;
	if ( m_remainingLives <= 0 )
	{
		m_game->SetGameState( GameState::GAME_STATE_ATTRACT );
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::AccelerateShip()
{
	if ( m_game->GetGameState() == GameState::GAME_STATE_ATTRACT )
	{
		return;
	}

	m_isShipAccelerating = true;
	m_thrustFraction = 1.f;
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::StopAcceleratingShip()
{
	m_isShipAccelerating = false;
	m_thrustFraction = 0.f;
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::Respawn()
{
	if ( m_game->GetGameState() == GameState::GAME_STATE_ATTRACT )
	{
		return;
	}

	if ( m_remainingLives > 0 )
	{
		// Reset member variables and return to starting position
		m_position = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y);
		m_linearAcceleration = Vec2( 0.f, 0.f );
		m_velocity = Vec2( 0.f, 0.f );
		m_angularVelocity = 0.f;
		m_orientationDegrees = 0.f;
		m_thrustFraction = 0.f;
		m_isDead = false;
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	
	if ( g_inputSystem->IsKeyPressed( 'W' )
		 || g_inputSystem->IsKeyPressed( KEY_UPARROW ) )
	{
		AccelerateShip();
	}
	else
	{
		StopAcceleratingShip();
	}

	if ( g_inputSystem->WasKeyJustReleased( KEY_SPACEBAR ) )
	{
		HandleBulletReload();
	}
	
	if ( g_inputSystem->IsKeyPressed( 'A' )
			  || g_inputSystem->IsKeyPressed( KEY_LEFTARROW ) )
	{
		TurnShipLeft();
	}
	else
	{
		StopTurningShipLeft();
	}
	
	if ( g_inputSystem->IsKeyPressed( 'D' )
			  || g_inputSystem->IsKeyPressed( KEY_RIGHTARROW ) )
	{
		TurnShipRight();
	}
	else
	{
		StopTurningShipRight();
	}
	
	if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
	{
		HandleStarburstBulletFired();
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerShip::UpdateFromGamepad( float deltaSeconds )
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

	// Start game if on attract screen, or respawn if player is dead when the start button is pressed
	if ( controller.WasButtonJustPressed(XBOX_BUTTON_ID_START) )
	{
		if ( m_game->GetGameState() == GameState::GAME_STATE_ATTRACT )
		{
			m_game->SetGameState(GameState::GAME_STATE_PLAY);
			return;
		}

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
		m_thrustFraction = leftStickMagnitude;
		m_isShipAcceleratingFromJoystick = true;
	}
	else
	{
		m_isShipAcceleratingFromJoystick = false;
	}

	if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_A ) )
	{
		HandleBulletFired();
		HandleBulletReload();
	}

	if ( controller.WasButtonJustPressed(XBOX_BUTTON_ID_X) )
	{
		HandleStarburstBulletFired();
	}
}
