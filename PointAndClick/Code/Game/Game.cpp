#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/UIButton.hpp"
#include "Game/UIPanel.hpp"


//-----------------------------------------------------------------------------------------------
SpriteSheet* g_tileSpriteSheet = nullptr;
SpriteSheet* g_characterSpriteSheet = nullptr;
SpriteSheet* g_portraitSpriteSheet = nullptr;


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
	m_worldCamera->SetOutputSize( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCamera->SetPosition( m_focalPoint );

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
	m_uiCamera->SetPosition( Vec3( WINDOW_WIDTH_PIXELS * .5f, WINDOW_HEIGHT_PIXELS * .5f, 0.f ) );

	EnableDebugRendering();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, true );

	BuildHUD();

	m_world = new World( m_gameClock );

	/*m_curMap = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMap );
	g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMap.c_str() ) );
	m_world->BuildNewMap( m_curMap );*/

	//LogMapDebugCommands();
	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::BeginFrame()
{
	
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	TileDefinition::s_definitions.clear();

	CleanupHUD();

	// Clean up global sprite sheets
	PTR_SAFE_DELETE( g_tileSpriteSheet );
	PTR_SAFE_DELETE( g_characterSpriteSheet );
	PTR_SAFE_DELETE( g_portraitSpriteSheet );
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::LogMapDebugCommands()
{
	g_devConsole->PrintString( "Map Generation Debug Commands" );
	g_devConsole->PrintString( "F4 - View entire map" );
	g_devConsole->PrintString( "F5 - Reload current map" );
}


////-----------------------------------------------------------------------------------------------
//void Game::SetWorldCameraOrthographicView( const AABB2& cameraBounds )
//{
//	m_worldCamera->SetOrthoView( cameraBounds.mins, cameraBounds.maxs );
//}
//
//
////-----------------------------------------------------------------------------------------------
//void Game::SetWorldCameraOrthographicView( const Vec2& bottomLeft, const Vec2& topRight )
//{
//	SetWorldCameraOrthographicView( AABB2( bottomLeft, topRight ) );
//}


//-----------------------------------------------------------------------------------------------
void Game::Update()
{
	switch ( m_gameState )
	{
		case eGameState::LOADING:
		{
			switch ( m_loadingFrameNum )
			{
				case 0:
				{
					++m_loadingFrameNum;

					SoundID anticipation = g_audioSystem->CreateOrGetSound( "Data/Audio/Anticipation.mp3" );
					g_audioSystem->PlaySound( anticipation, false, .25f );
				}
				break;

				case 1:
				{
					LoadAssets();
					ChangeGameState( eGameState::ATTRACT );
					Update();
				}
				break;
			}
		}
		break;

		case eGameState::ATTRACT:
		{
			UpdateFromKeyboard();
		}
		break;

		case eGameState::PLAYING:
		{
			UpdateFromKeyboard();

			m_world->Update();
		}
		break;
	}

	UpdateCameras();
	UpdateMousePositions();

	m_rootPanel->Update();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_gameState )
	{
		case eGameState::PLAYING:
		case eGameState::PAUSED:
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

	g_renderer->BeginCamera( *m_uiCamera );

	switch ( m_gameState )
	{
		case eGameState::LOADING:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Loading..." );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;

		case eGameState::ATTRACT:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Protogame2D" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 400.f ), 30.f, "Press Any Key to Start" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;

		case eGameState::PLAYING:
		{
			//m_world->RenderHUD();
		}
		break;
	}

	m_rootPanel->Render( g_renderer );

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Game::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );
	g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );

	// Music
	g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/GameOver.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheScummBar.mp3" );

	// TODO: Check for nullptrs when loading textures
	g_tileSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_32x32.png" )), IntVec2( 32, 32 ) );
	g_characterSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaCharacters_12x53.png" )), IntVec2( 12, 53 ) );
	g_portraitSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaPortraits_8x8.png" )), IntVec2( 8, 8 ) );

	g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" );
	g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/DebugRender.hlsl" );

	LoadTilesFromXml();
	LoadMapsFromXml();
	LoadActorsFromXml();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTilesFromXml()
{
	g_devConsole->PrintString( "Loading Tiles..." );

	const char* filePath = "Data/Gameplay/TileDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The tiles xml file '%s' could not be opened.", filePath ) );
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		TileDefinition* tileDef = new TileDefinition( *element );
		TileDefinition::s_definitions[ tileDef->GetName() ] = tileDef;

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Tiles Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadMapsFromXml()
{
	g_devConsole->PrintString( "Loading Maps..." );

	const char* filePath = "Data/Gameplay/MapDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The maps xml file '%s' could not be opened.", filePath ) );
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		MapDefinition* mapDef = new MapDefinition( *element );
		MapDefinition::s_definitions[ mapDef->GetName() ] = mapDef;

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadActorsFromXml()
{
	g_devConsole->PrintString( "Loading Actors..." );

	const char* filePath = "Data/Gameplay/ActorDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The actors xml file '%s' could not be opened.", filePath ) );
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		ActorDefinition* actorDef = new ActorDefinition( *element );
		ActorDefinition::s_definitions[actorDef->GetName()] = actorDef;

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Actors Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			if ( g_inputSystem->WasAnyKeyJustPressed() )
			{
				ChangeGameState( eGameState::PLAYING );
			}
		}
		break;

		case eGameState::PLAYING:
		{
			m_isSlowMo = g_inputSystem->IsKeyPressed( 'T' );
			m_isFastMo = g_inputSystem->IsKeyPressed( 'Y' );

			if ( g_inputSystem->WasKeyJustPressed( 'P' ) )
			{
				m_isPaused = !m_isPaused;
			}

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
				LoadNewMap( m_curMap );
			}
		}
		break;

	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadNewMap( const std::string& mapName )
{
	delete m_world;
	m_world = nullptr;
	m_world = new World( m_gameClock );
	m_world->BuildNewMap( mapName );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMousePositions()
{
	UpdateMouseWorldPosition();
	UpdateMouseUIPosition();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseWorldPosition()
{
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_worldCamera->GetOutputSize();
	m_mouseWorldPosition += m_worldCamera->GetOrthoMin();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseUIPosition()
{
	m_mouseUIPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_uiCamera->GetOutputSize();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * (float)m_gameClock->GetLastDeltaSeconds();
	m_screenShakeIntensity = ClampZeroToOne( m_screenShakeIntensity );

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	float cameraShakeY = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	Vec2 cameraShakeOffset = Vec2( cameraShakeX, cameraShakeY );

	//m_worldCamera->Translate2D( cameraShakeOffset );
	m_worldCamera->SetPosition( m_focalPoint + Vec3( cameraShakeOffset, 0.f ) );
	m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT );

	// UI Camera
	m_uiCamera->SetPosition( Vec3( WINDOW_WIDTH_PIXELS * .5f, WINDOW_HEIGHT_PIXELS * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( WINDOW_HEIGHT_PIXELS );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::SetWorldCameraPosition( const Vec3& position )
{
	m_focalPoint = position;
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity(float intensity)
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void Game::PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines )
{
	if ( (int)textLines.size() == 0 )
	{
		return;
	}

	m_debugInfoTextBox->SetText( textLines[0], color );

	for ( int textLineIndex = 1; textLineIndex < (int)textLines.size(); ++textLineIndex )
	{
		m_debugInfoTextBox->AddLineOFText( textLines[ textLineIndex ], color );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeGameState( const eGameState& newGameState )
{
	switch ( newGameState )
	{
		case eGameState::LOADING:
		{
			ERROR_AND_DIE( "Tried to go back to the loading state during the game. Don't do that." );
		}
		break;

		case eGameState::ATTRACT:
		{
			// Check which state we are changing from
			switch ( m_gameState )
			{
				case eGameState::PAUSED:
				case eGameState::PLAYING:
				{
					g_audioSystem->StopSound( m_gameplayMusicID );
				}
				break;

				case eGameState::VICTORY:
				{
					g_audioSystem->StopSound( m_victoryMusicID );
				}
				break;
			}

			SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
			m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true, .25f );
		}
		break;

		case eGameState::PLAYING:
		{
			// Check which state we are changing from
			switch ( m_gameState )
			{
				case eGameState::PAUSED:
				{
					SoundID unpause = g_audioSystem->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
					g_audioSystem->PlaySound( unpause );

					g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, 1.f );
				}
				break;

				case eGameState::ATTRACT:
				{
					g_audioSystem->StopSound( m_attractMusicID );

					SoundID gameplayMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheScummBar.mp3" );
					m_gameplayMusicID = g_audioSystem->PlaySound( gameplayMusic, true, .25f );
					
					m_curMap = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMap );
					g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMap.c_str() ) );
					m_world->BuildNewMap( m_curMap );
				}
				break;
			}
		}
		break;

		case eGameState::PAUSED:
		{
			g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, .5f );

			SoundID pause = g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
			g_audioSystem->PlaySound( pause );

		}
		break;

		case eGameState::VICTORY:
		{
			//m_curVictoryScreenSeconds = 0.f;

			g_audioSystem->StopSound( m_gameplayMusicID );

			SoundID victoryMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
			m_victoryMusicID = g_audioSystem->PlaySound( victoryMusic );
		}
		break;
	}

	m_gameState = newGameState;
}


//-----------------------------------------------------------------------------------------------
void Game::BuildHUD()
{
	//Texture* rootBackground = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	Texture* childBackground = g_renderer->GetDefaultWhiteTexture();

	m_rootPanel = new UIPanel( AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ) );

	m_hudPanel = new UIPanel( m_rootPanel, Vec2( 0.f, 1.f ), Vec2( 0.f, .25f ), childBackground );
	m_hudPanel->SetTint( Rgba8::RED );

	BuildVerbPanel();
		
	m_rootPanel->AddChildPanel( *m_hudPanel );
}


//-----------------------------------------------------------------------------------------------
void Game::CleanupHUD()
{
	PTR_SAFE_DELETE( m_giveVerbButton );
	PTR_SAFE_DELETE( m_openVerbButton );
	PTR_SAFE_DELETE( m_closeVerbButton );
	PTR_SAFE_DELETE( m_pickUpVerbButton );
	PTR_SAFE_DELETE( m_talkToVerbButton );

	PTR_SAFE_DELETE( m_verbPanel );
	PTR_SAFE_DELETE( m_inventoryPanel );
	PTR_SAFE_DELETE( m_hudPanel );
	PTR_SAFE_DELETE( m_dialoguePanel );
	PTR_SAFE_DELETE( m_rootPanel );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildVerbPanel()
{
	Texture* background = g_renderer->GetDefaultWhiteTexture();

	m_verbPanel = new UIPanel( m_hudPanel, Vec2( 0.f, .5f ), Vec2( 0.f, 1.f ), background );
	m_verbPanel->SetTint( Rgba8::BLACK );

	m_giveVerbButton = new UIButton( *m_verbPanel, Vec2( 0.f, 0.5f ), Vec2( 0.33f, .5f ), background );
	m_giveVerbButton->SetTint( Rgba8::RED );
	m_giveVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_verbPanel->AddButton( *m_giveVerbButton );

	m_openVerbButton = new UIButton( *m_verbPanel, Vec2( .33f, 0.5f ), Vec2( 0.33f, .5f ), background );
	m_openVerbButton->SetTint( Rgba8::ORANGE );
	m_openVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_verbPanel->AddButton( *m_openVerbButton );

	m_closeVerbButton = new UIButton( *m_verbPanel, Vec2( .66f, 0.5f ), Vec2( 0.34f, .5f ), background );
	m_closeVerbButton->SetTint( Rgba8::YELLOW );
	m_closeVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_verbPanel->AddButton( *m_closeVerbButton );

	m_pickUpVerbButton = new UIButton( *m_verbPanel, Vec2( 0.f, 0.f ), Vec2( 0.5f, .5f ), background );
	m_pickUpVerbButton->SetTint( Rgba8::GREEN );
	m_pickUpVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_verbPanel->AddButton( *m_pickUpVerbButton );

	m_talkToVerbButton = new UIButton( *m_verbPanel, Vec2( .5f, 0.f ), Vec2( 0.5f, .5f ), background );
	m_talkToVerbButton->SetTint( Rgba8::BLUE );
	m_talkToVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_verbPanel->AddButton( *m_talkToVerbButton );

	m_hudPanel->AddChildPanel( *m_verbPanel );
}


//-----------------------------------------------------------------------------------------------
// Button events
//-----------------------------------------------------------------------------------------------
void Game::OnTestButtonClicked( EventArgs* args )
{
	UNUSED( args );

	/*SoundID anticipation = g_audioSystem->CreateOrGetSound( "Data/Audio/Anticipation.mp3" );
	g_audioSystem->PlaySound( anticipation, false, .25f );*/

	//Mat44::CreateTranslation2D( m_testButton->GetPosition() + Vec2( 0.f, 1.f );
	DebugAddWorldTextf( Mat44::CreateTranslation2D( Vec2( 5.f, 5.f ) ),
						Vec2( .5f, .5f ),
						Rgba8::GREEN,
						1.f,
						.1f,
						DEBUG_RENDER_ALWAYS,
						"Button clicked!");
}
