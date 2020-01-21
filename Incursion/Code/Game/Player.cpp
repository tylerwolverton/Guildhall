#include "Player.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/RaycastImpact.hpp"


//-----------------------------------------------------------------------------------------------
Player::Player( Map* map, EntityFaction faction, const Vec2& position )
	: Entity( map, ENTITY_TYPE_PLAYER, faction, position )
	, m_startingPosition( position )
{
	m_physicsRadius = PLAYER_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_COSMETIC_RADIUS;
	m_maxTurnSpeed = PLAYER_MAX_TURN_SPEED;
	m_maxHealth = m_health = PLAYER_MAX_HEALTH;

	m_diedSound = g_audioSystem->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	m_hitSound = g_audioSystem->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	m_shootSound = g_audioSystem->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );

	m_pushesEntities = true;
	m_isPushedByEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
Player::~Player()
{
}


//-----------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	UpdateInput( deltaSeconds );

	if ( IsDead() )
	{
		m_curDeathSeconds += deltaSeconds;
		return;
	}
	if ( m_curDeathDismissalSeconds > 0.f )
	{
		m_curDeathDismissalSeconds -= deltaSeconds;
	}

	Entity::Update( deltaSeconds );

	m_velocity.ClampLength( PLAYER_MAX_SPEED );
}


//-----------------------------------------------------------------------------------------------
void Player::Render() const
{
	if ( m_isDead )
	{
		return;
	}

	// Tank body
	std::vector<Vertex_PCU> vertexesCopy( m_vertexes );
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, m_orientationDegrees, m_position );

	g_renderer->BindTexture( m_tankBodyTexture );
	g_renderer->DrawVertexArray( vertexesCopy );

	// Tank gun
	vertexesCopy = m_vertexes;
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, m_orientationDegrees + m_gunOrientationDegrees, m_position );

	g_renderer->BindTexture( m_tankGunTexture );
	g_renderer->DrawVertexArray( vertexesCopy );

	RenderHealthBar();
}


//-----------------------------------------------------------------------------------------------
void Player::RenderHUD() const
{
	if ( m_isDead )
	{
		g_game->RenderDeathOverlay( m_curDeathSeconds );
	}
	else if ( m_curDeathDismissalSeconds > 0.f )
	{
		g_game->RenderDeathOverlayFadeOut( m_curDeathDismissalSeconds );
	}

	std::vector<Vertex_PCU> textVertexes;
	std::string text( "Deaths: " + std::to_string( m_numDeaths ) );
	m_font->AppendVertsForText2D( textVertexes, Vec2( 10.f, WINDOW_HEIGHT - 1.f ), 25.f, text );

	g_renderer->BindTexture( m_font->GetTexture() );
	g_renderer->DrawVertexArray( textVertexes );
}


//-----------------------------------------------------------------------------------------------
void Player::DebugRender() const
{
	Entity::DebugRender();

	// Raycast debug
	RaycastImpact impact = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( GetPlayerGunWorldOrientation() ), 3.f );
	g_renderer->BindTexture( nullptr );
	g_renderer->DrawLine2D( m_position, impact.m_impactPosition, Rgba8::WHITE, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
void Player::Die()
{
	m_isDead = true;
	m_curDeathSeconds = 0.f; 
	++m_numDeaths;

	g_audioSystem->PlaySound( m_diedSound );
	m_map->SpawnExplosion( m_position, 2.f, 2.5f );
}


//-----------------------------------------------------------------------------------------------
void Player::TakeDamage( int damage )
{
	Entity::TakeDamage( damage );

	g_audioSystem->PlaySound( m_hitSound );
}


//-----------------------------------------------------------------------------------------------
void Player::UpdateInput( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );
	UpdateFromGamepad( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Player::Respawn()
{
	m_isDead = false;
	m_health = PLAYER_MAX_HEALTH;
	m_velocity = Vec2::ZERO;

	m_curDeathDismissalSeconds = DEATH_OVERLAY_DISMISSAL_TIMER_SECONDS;
}


//-----------------------------------------------------------------------------------------------
void Player::PopulateVertexes()
{
	m_font = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	m_tankBodyTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank0.png" );
	m_tankGunTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyCannon.png" );

	g_renderer->AppendVertsForAABB2D( m_vertexes, AABB2::ONE_BY_ONE, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
}


//-----------------------------------------------------------------------------------------------
void Player::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	switch ( g_game->GetGameState() )
	{
		case GAME_STATE_ATTRACT:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_app->HandleQuitRequested();
			}
			else if ( g_inputSystem->WasKeyJustPressed( 'P' ) )
			{
				g_game->ChangeGameState( GAME_STATE_PLAYING );
			}
		}
		break;

		case GAME_STATE_PLAYING:
		{
			if ( m_isDead )
			{
				if ( m_curDeathSeconds > DEATH_OVERLAY_TIMER_SECONDS )
				{
					if ( g_inputSystem->IsKeyPressed( 'P' ) )
					{
						Respawn();
					}
					else if ( g_inputSystem->IsKeyPressed( KEY_ESC ) )
					{
						g_game->ChangeGameState( GAME_STATE_ATTRACT );
					}
				}
				return;
			}

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

			if ( g_inputSystem->WasKeyJustPressed( KEY_SPACEBAR ) )
			{
				FireBullet();
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) 
				 || g_inputSystem->WasKeyJustPressed( 'P' ) )
			{
				g_game->ChangeGameState( GAME_STATE_PAUSED );
			}
		}
		break;

		case GAME_STATE_PAUSED:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_game->ChangeGameState( GAME_STATE_ATTRACT );
			}
			else if ( g_inputSystem->WasKeyJustPressed( 'P' ) )
			{
				g_game->ChangeGameState( GAME_STATE_PLAYING );
			}
		}
		break;

		case GAME_STATE_VICTORY:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) 
				 || g_inputSystem->WasKeyJustPressed( 'P' ) )
			{
				g_game->ChangeGameState( GAME_STATE_ATTRACT );
			}
		}
		break;
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
	
	switch ( g_game->GetGameState() )
	{
		case GAME_STATE_PLAYING:
		{
			if ( m_isDead )
			{
				if ( m_curDeathSeconds > DEATH_OVERLAY_TIMER_SECONDS )
				{
					if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_START ) )
					{
						Respawn();
					}
					else if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_BACK ) )
					{
						g_game->ChangeGameState( GAME_STATE_ATTRACT );
					}
				}
				return;
			}

			if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_START ) )
			{
				g_game->ChangeGameState( GAME_STATE_PAUSED );
				return;
			}

			// Move tank with left joystick
			const AnalogJoystick& leftStick = controller.GetLeftJoyStick();
			float leftStickMagnitude = leftStick.GetMagnitude();
			if ( leftStickMagnitude > 0.f )
			{
				TurnToward( leftStick.GetDegrees(), deltaSeconds );
				m_velocity += leftStickMagnitude * PLAYER_SPEED * GetForwardVector();
			}

			// Move tank gun with right joystick
			const AnalogJoystick& rightStick = controller.GetRightJoyStick();
			float rightStickMagnitude = rightStick.GetMagnitude();
			if ( rightStickMagnitude > 0.f )
			{
				float deltaDegrees = PLAYER_GUN_TURN_SPEED * deltaSeconds;

				m_gunOrientationDegrees = GetTurnedToward( m_gunOrientationDegrees, rightStick.GetDegrees() - m_orientationDegrees, deltaDegrees );
			}

			if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_A ) )
			{
				FireBullet();
			}
		}
		break;

		case GAME_STATE_ATTRACT:
		{
			if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_START ) )
			{
				g_game->ChangeGameState( GAME_STATE_PLAYING );
			}
			else if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_BACK ) )
			{
				g_app->HandleQuitRequested();
			}
		}
		break;

		case GAME_STATE_PAUSED:
		{
			if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_START ) )
			{
				g_game->ChangeGameState( GAME_STATE_PLAYING );
			}
			else if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_BACK ) )
			{
				m_isDead = true;
				g_game->ChangeGameState( GAME_STATE_ATTRACT );
			}
		}
		break;

		case GAME_STATE_VICTORY:
		{
			if ( controller.WasButtonJustPressed( XBOX_BUTTON_ID_START )
				|| controller.WasButtonJustPressed( XBOX_BUTTON_ID_BACK ) )
			{
				g_game->ChangeGameState( GAME_STATE_ATTRACT );
			}
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Player::FireBullet()
{
	Vec2 spawnPos = m_position + ( PLAYER_GUN_COSMETIC_RADIUS * Vec2::MakeFromPolarDegrees( GetPlayerGunWorldOrientation() ) );
	m_map->SpawnNewEntity( ENTITY_TYPE_GOOD_BULLET, m_faction, spawnPos, GetPlayerGunWorldOrientation() );

	g_audioSystem->PlaySound( m_shootSound );
}


//-----------------------------------------------------------------------------------------------
float Player::GetPlayerGunWorldOrientation() const
{
	return m_orientationDegrees + m_gunOrientationDegrees;
}
