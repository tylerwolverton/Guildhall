#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_worldCamera = new Camera();
	m_uiCamera = new Camera();

	m_rng = new RandomNumberGenerator();

	PopulateFullScreenVertexes();

	m_world = new World();
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_world;
	m_world = nullptr;

	delete m_rng;
	m_rng = nullptr;
	
	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Game::StopAllMusic()
{
	if ( m_attractMusicID != -1 )
	{
		g_audioSystem->StopSound( m_attractMusicID );
	}
	if ( m_gameplayMusicID != -1 )
	{
		g_audioSystem->StopSound( m_gameplayMusicID );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::SetWorldCameraOrthographicView( const AABB2& cameraBounds )
{
	m_worldCamera->SetOrthoView( cameraBounds.mins, cameraBounds.maxs );
}


//-----------------------------------------------------------------------------------------------
void Game::SetWorldCameraOrthographicView( const Vec2& bottomLeft, const Vec2& topRight )
{
	SetWorldCameraOrthographicView( AABB2( bottomLeft, topRight ) );
}


//-----------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	switch ( m_gameState )
	{
		case GAME_STATE_LOADING:
		{
			switch ( m_loadingFrameNum )
			{
				case 0:
				{
					++m_loadingFrameNum;

					m_font = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

					SoundID anticipation = g_audioSystem->CreateOrGetSound( "Data/Audio/Anticipation.mp3" );
					g_audioSystem->PlaySound( anticipation );
				}
				break;

				case 1:
				{
					LoadAssets();
					ChangeGameState( GAME_STATE_ATTRACT );					
					Update( deltaSeconds );
				}
				break;
			}
		}
		break;

		case GAME_STATE_ATTRACT:
		{
			if ( m_needToGenerateMaps )
			{
				GenerateMaps();
				m_needToGenerateMaps = false;
			}

			UpdateFromKeyboard( deltaSeconds );

			Player* player = m_world->GetPlayer();
			if ( player != nullptr )
			{
				player->UpdateInput( deltaSeconds );
			}
		}
		break;

		case GAME_STATE_PLAYING:
		{
			UpdateFromKeyboard( deltaSeconds );

			// Modify deltaSeconds based on game state
			if ( m_isSlowMo )
			{
				deltaSeconds *= .1f;
			}
			if ( m_isFastMo )
			{
				deltaSeconds *= 4.f;
			}

			m_world->Update( deltaSeconds );
		}
		break;

		case GAME_STATE_PAUSED:
		{
			UpdateFromKeyboard( deltaSeconds );

			deltaSeconds = 0.f;
			Player* player = m_world->GetPlayer();
			if ( player != nullptr )
			{
				player->UpdateInput( deltaSeconds );
			}
		}
		break;

		case GAME_STATE_VICTORY:
		{
			m_curEndGameSeconds -= deltaSeconds;
			m_curVictoryScreenSeconds += deltaSeconds;

			if ( m_curEndGameSeconds <= 0.f )
			{
				UpdateFromKeyboard( deltaSeconds );

				Player* player = m_world->GetPlayer();
				if ( player != nullptr )
				{
					player->UpdateInput( deltaSeconds );
				}
			}
		}
		break;
	}

	UpdateCameras( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_renderer->ClearScreen(Rgba8::BLACK);

	g_renderer->BeginCamera(*m_worldCamera );

	switch ( m_gameState )
	{
		case GAME_STATE_PLAYING:
		case GAME_STATE_PAUSED:
		{
			m_world->Render();
			if ( m_isDebugRendering )
			{
				m_world->DebugRender();
			}
		}
		break;
	}

	g_renderer->EndCamera( *m_worldCamera );

	// Render UI with a new camera
	g_renderer->BeginCamera( *m_uiCamera );
	
	switch ( m_gameState )
	{
		case GAME_STATE_LOADING:
		{
			std::vector<Vertex_PCU> vertexes;
			m_font->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Loading..." );

			g_renderer->BindTexture( m_font->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;
		
		case GAME_STATE_ATTRACT:
		{
			std::vector<Vertex_PCU> vertexes;
			m_font->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Incursion" );

			g_renderer->BindTexture( m_font->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;
		
		case GAME_STATE_PLAYING:
		{
			m_world->RenderHUD();
		}
		break;

		case GAME_STATE_PAUSED:
		{
			m_world->RenderHUD();
			RenderPauseOverlay();
		}
		break;

		case GAME_STATE_VICTORY:
		{
			RenderVictoryScreen();
		}
		break;
	}

	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderPauseOverlay() const
{
	g_renderer->BindTexture( nullptr );
	g_renderer->DrawVertexArray( m_darkScreenVertexes );

	std::vector<Vertex_PCU> vertexes;
	m_font->AppendVertsForText2D( vertexes, Vec2( 750.f, 500.f ), 50.f, "Paused..." );
	m_font->AppendVertsForText2D( vertexes, Vec2( 450.f, 450.f ), 24.f, "Press P or Start to continue, Back or Esc to exit" );

	g_renderer->BindTexture( m_font->GetTexture() );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderDeathOverlay( float curDeathSeconds ) const
{
	if ( curDeathSeconds > DEATH_OVERLAY_TIMER_SECONDS )
	{
		g_renderer->BindTexture( nullptr );
		g_renderer->DrawVertexArray( m_darkScreenVertexes );

		std::vector<Vertex_PCU> vertexes;
		m_font->AppendVertsForText2D( vertexes, Vec2( 750.f, 500.f ), 50.f, "You died." );
		m_font->AppendVertsForText2D( vertexes, Vec2( 450.f, 450.f ), 24.f, "Press P or Start to respawn, Back or Esc to exit" );

		g_renderer->BindTexture( m_font->GetTexture() );
		g_renderer->DrawVertexArray( vertexes );
	}
	else
	{
		unsigned char newAlpha = (unsigned char)(curDeathSeconds * 100.f / DEATH_OVERLAY_TIMER_SECONDS);

		std::vector<Vertex_PCU> vertexesCopy( m_darkScreenVertexes );
		for ( int vertIndex = 0; vertIndex < (int)vertexesCopy.size(); ++vertIndex )
		{
			vertexesCopy[vertIndex].m_color.a = newAlpha;
		}

		g_renderer->BindTexture( nullptr );
		g_renderer->DrawVertexArray( vertexesCopy );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RenderDeathOverlayFadeOut( float curDeathSeconds ) const
{
	unsigned char newAlpha = (unsigned char)( curDeathSeconds * 100.f / DEATH_OVERLAY_DISMISSAL_TIMER_SECONDS );

	std::vector<Vertex_PCU> vertexesCopy( m_darkScreenVertexes );
	for ( int vertIndex = 0; vertIndex < (int)vertexesCopy.size(); ++vertIndex )
	{
		vertexesCopy[vertIndex].m_color.a = newAlpha;
	}

	g_renderer->BindTexture( nullptr );
	g_renderer->DrawVertexArray( vertexesCopy );

	std::vector<Vertex_PCU> vertexes;
	m_font->AppendVertsForText2D( vertexes, Vec2( 750.f, 500.f ), 50.f, "You died." );
	m_font->AppendVertsForText2D( vertexes, Vec2( 450.f, 450.f ), 24.f, "Press P or Start to respawn, Back or Esc to exit" ); 
	
	g_renderer->BindTexture( m_font->GetTexture() );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderVictoryScreen() const
{
		std::vector<Vertex_PCU> vertexCopy( m_darkScreenVertexes );
		for ( int vertexIndex = 0; vertexIndex < (int)vertexCopy.size(); ++vertexIndex )
		{
			vertexCopy[vertexIndex].m_color = Rgba8::GREEN;
		}

		if ( m_curVictoryScreenSeconds < VICTORY_OVERLAY_TIMER_SECONDS )
		{
			unsigned char newAlpha = (unsigned char)( m_curVictoryScreenSeconds * 255.f / VICTORY_OVERLAY_TIMER_SECONDS );

			for ( int vertIndex = 0; vertIndex < (int)vertexCopy.size(); ++vertIndex )
			{
				vertexCopy[vertIndex].m_color.a = newAlpha;
			}
		}

		g_renderer->BindTexture( nullptr );
		g_renderer->DrawVertexArray( vertexCopy );

		std::vector<Vertex_PCU> victoryVertexes;
		m_font->AppendVertsForText2D( victoryVertexes, Vec2( 650.f, 500.f ), 70.f, "You win!", Rgba8::BLACK );

		g_renderer->BindTexture( m_font->GetTexture() );
		g_renderer->DrawVertexArray( victoryVertexes );

		if ( m_curEndGameSeconds <= 0.f )
		{
			std::vector<Vertex_PCU> instructionVertexes;
			m_font->AppendVertsForText2D( instructionVertexes, Vec2( 550.f, 440.f ), 20.f, "Press Start, Back, P, or Esc to Exit", Rgba8::BLACK );

			g_renderer->DrawVertexArray( instructionVertexes );
		}
}


//-----------------------------------------------------------------------------------------------
void Game::GenerateMaps()
{
	m_world->GenerateMaps();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	switch ( m_gameState )
	{
		case GAME_STATE_PLAYING:
		{
			m_isSlowMo = g_inputSystem->IsKeyPressed( 'T' );
			m_isFastMo = g_inputSystem->IsKeyPressed( 'Y' );
			
			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
			{
				m_isNoClipEnabled = !m_isNoClipEnabled;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
			{
				m_isDebugCameraEnabled = !m_isDebugCameraEnabled;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
			{
				m_world->ChangeMap();
			}
		}
		break;

		case GAME_STATE_PAUSED:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
			{
				m_isNoClipEnabled = !m_isNoClipEnabled;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
			{
				m_isDebugCameraEnabled = !m_isDebugCameraEnabled;
			}
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * deltaSeconds;
	m_screenShakeIntensity = ClampMinMax(m_screenShakeIntensity, 0.f, 1.0);

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_rng->RollRandomFloatInRange(-maxScreenShake, maxScreenShake);
	float cameraShakeY = m_rng->RollRandomFloatInRange(-maxScreenShake, maxScreenShake);
	Vec2 cameraShakeOffset = Vec2(cameraShakeX, cameraShakeY);

	m_worldCamera->Translate2D(cameraShakeOffset);

	// UI Camera
	m_uiCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	LoadImages();
	LoadAudio();
}


//-----------------------------------------------------------------------------------------------
void Game::LoadImages()
{
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBullet.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyCannon.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyGatling.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank0.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank1.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank2.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank3.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyCannon.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyGatling.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyShell.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank0.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank1.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank2.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank3.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTank4.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyTurretBase.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/GatlingBullet.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	// Music
	g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/GameOver.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/GameplayMusic.mp3" );

	// Sounds
	g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/PlayerDied.wav" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/PlayerHit.wav" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/PlayerShootNormal.ogg" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/MapExited.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/QuitGame.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/StartGame.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Unpause.mp3" );

}


//-----------------------------------------------------------------------------------------------
void Game::PopulateFullScreenVertexes()
{
	AABB2 bounds( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
	Rgba8 color( Rgba8::BLACK );
	color.a = 100;

	g_renderer->AppendVertsForAABB2D( m_darkScreenVertexes, bounds, color, Vec2::ZERO, Vec2::ONE );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity(float intensity)
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeGameState( GameState newGameState )
{
	switch ( newGameState )
	{
		case GAME_STATE_LOADING:
		{
			ERROR_AND_DIE( "Tried to go back to the loading state during the game. Don't do that." );
		}
		break;
		
		case GAME_STATE_ATTRACT:
		{
			m_needToGenerateMaps = true;

			// Check which state we are changing from
			switch ( m_gameState )
			{
				case GAME_STATE_PAUSED:
				case GAME_STATE_PLAYING:
				{
					g_audioSystem->StopSound( m_gameplayMusicID );
				}
				break;

				case GAME_STATE_VICTORY:
				{
					g_audioSystem->StopSound( m_victoryMusicID );
				}
				break;
			}
			
			SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
			m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true );
		}
		break;
		
		case GAME_STATE_PLAYING:
		{
			// Check which state we are changing from
			switch ( m_gameState )
			{
				case GAME_STATE_PAUSED:
				{
					SoundID unpause = g_audioSystem->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
					g_audioSystem->PlaySound( unpause );

					g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, 1.f );
				}
				break;

				case GAME_STATE_ATTRACT:
				{
					g_audioSystem->StopSound( m_attractMusicID );

					SoundID gameplayMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/GameplayMusic.mp3" );
					m_gameplayMusicID = g_audioSystem->PlaySound( gameplayMusic, true );
				}
				break;
			}
		}
		break;
		
		case GAME_STATE_PAUSED:
		{
			g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, .5f );

			SoundID pause = g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
			g_audioSystem->PlaySound( pause );

		}
		break;
		
		case GAME_STATE_VICTORY:
		{
			m_curVictoryScreenSeconds = 0.f;

			g_audioSystem->StopSound( m_gameplayMusicID );

			SoundID victoryMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
			m_victoryMusicID = g_audioSystem->PlaySound( victoryMusic );
		}
		break;
	}

	m_gameState = newGameState;
}
