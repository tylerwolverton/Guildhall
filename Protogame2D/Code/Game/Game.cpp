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
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/Actor.hpp"
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapData.hpp"


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

	Vec2 windowDimensions = g_window->GetDimensions();

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );

	EnableDebugRendering();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );

	m_world = new World( m_gameClock );

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

	m_uiSystem->Shutdown();

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
					g_audioSystem->PlaySound( anticipation, false, .1f );
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

	m_uiSystem->Update();
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
	}

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

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
	g_audioSystem->CreateOrGetSound( "Data/Audio/GameplayMusic.mp3" );

	LoadEntitiesFromXml();
	LoadTileMaterialsFromXml();
	LoadTilesFromXml();
	LoadMapsFromXml();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTileMaterialsFromXml()
{
	g_devConsole->PrintString( "Loading Tile Materials..." );

	const char* filePath = "Data/Gameplay/TileMaterialDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "Data/Gameplay/TileMaterialDefs.xml: Could not be opened." );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "TileMaterialDefinitions" ) )
	{
		g_devConsole->PrintError( "TileMaterialDefs.xml: Incorrect root node name, must be TileMaterialDefinitions" );
		return;
	}

	// Parse spritesheet
	std::string spriteSheetPath = ParseXmlAttribute( *root, "spriteSheet", "" );
	if ( spriteSheetPath == "" )
	{
		g_devConsole->PrintError( "TileMaterialDefs.xml: Missing a spriteSheet attribute" );
		return;
	}
	
	SpriteSheet* spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
	if ( spriteSheet == nullptr )
	{
		IntVec2 spriteSheetDimensions( -1, -1 );
		spriteSheetDimensions = ParseXmlAttribute( *root, "spriteSheetDimensions", spriteSheetDimensions );
		if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
		{
			g_devConsole->PrintError( "TileMaterialDefs.xml: Missing a spriteSheetDimensions attribute" );
			return;
		}

		Texture* texture = g_renderer->CreateOrGetTextureFromFile( ( "Data/" + spriteSheetPath ).c_str() );
		if ( texture == nullptr )
		{
			g_devConsole->PrintError( Stringf( "TileMaterialDefs.xml: Couldn't load texture '%s'", spriteSheetPath.c_str() ) );
			return;
		}
	
		spriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		TileMaterialDefinition* tileMatDef = new TileMaterialDefinition( *element, spriteSheet );

		if ( tileMatDef->IsValid() )
		{
			TileMaterialDefinition::s_definitions[tileMatDef->GetName()] = tileMatDef;
		}
		else
		{
			PTR_SAFE_DELETE( tileMatDef );
		}

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Tile Materials Loaded", Rgba8::GREEN );
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
		g_devConsole->PrintError( Stringf( "The tiles xml file '%s' could not be opened.", filePath ) );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "TileDefinitions" ) )
	{
		g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be TileDefinitions", filePath ) );
		return;
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		TileDefinition* tileDef = new TileDefinition( *element, m_defaultTileMaterialName );

		if ( tileDef->IsValid() )
		{
			TileDefinition::s_definitions[tileDef->GetName()] = tileDef;
		}
		else
		{
			PTR_SAFE_DELETE( tileDef );
		}

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Tiles Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadMapsFromXml()
{
	g_devConsole->PrintString( "Loading Maps..." );

	std::string folderPath( "Data/Maps" );

	Strings mapFiles = GetFileNamesInFolder( folderPath, "*.xml" );
	for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
	{
		std::string& mapName = mapFiles[mapIdx];

		std::string mapFullPath( folderPath );
		mapFullPath += "/";
		mapFullPath += mapName;

		XmlDocument doc;
		XmlError loadError = doc.LoadFile( mapFullPath.c_str() );
		if ( loadError != tinyxml2::XML_SUCCESS )
		{
			g_devConsole->PrintError( Stringf( "'%s' could not be opened", mapFullPath.c_str() ) );
			continue;
		}

		XmlElement* root = doc.RootElement();
		if ( strcmp( root->Name(), "MapDefinition" ) )
		{
			g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be MapDefinition", mapFullPath.c_str() ) );
			return;
		}

		MapData mapData( *root, GetFileNameWithoutExtension( mapName ), m_defaultTileName );

		if ( mapData.isValid )
		{
			m_world->AddNewMap( mapData );
		}
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadEntitiesFromXml()
{
	g_devConsole->PrintString( "Loading Entity Types..." );

	const char* filePath = "Data/Gameplay/EntityTypes.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "EntityTypes.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "EntityTypes" ) )
	{
		g_devConsole->PrintError( "EntityTypes.xml: Incorrect root node name, must be EntityTypes" );
		return;
	}

	// Parse sprite sheet
	std::string spriteSheetPath = ParseXmlAttribute( *root, "spriteSheet", "" );
	if ( spriteSheetPath == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: Missing spriteSheet attribute" );
		return;
	}

	SpriteSheet* spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
	if ( spriteSheet == nullptr )
	{
		IntVec2 spriteSheetDimensions = ParseXmlAttribute( *root, "spriteSheetDimensions", IntVec2( -1, -1 ) );
		if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
		{
			g_devConsole->PrintError( "EntityTypes.xml: Missing layout attribute" );
			return;
		}

		Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
		if ( texture == nullptr )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml: Couldn't load texture '%s'", spriteSheetPath.c_str() ) );
			return;
		}

		spriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		if ( !strcmp( element->Name(), "Actor" )
			 || !strcmp( element->Name(), "Entity" )
			 || !strcmp( element->Name(), "Projectile" )
			 || !strcmp( element->Name(), "Portal" ) )
		{
			EntityDefinition* entityTypeDef = new EntityDefinition( *element, spriteSheet );
			if ( entityTypeDef->IsValid() )
			{
				EntityDefinition::s_definitions[entityTypeDef->GetName()] = entityTypeDef;
			}
		}
		else
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported node '%s'", element->Name() ) );
		}

		element = element->NextSiblingElement();
	}

	std::string playerActorName = g_gameConfigBlackboard.GetValue( std::string( "playerActorName" ), "" );
	if ( playerActorName.empty() )
	{
		g_devConsole->PrintError( "GameConfig.xml doesn't define a playerActorName" );
		return;
	}

	EntityDefinition* playerDef = EntityDefinition::GetEntityDefinition( playerActorName );
	if ( playerDef == nullptr )
	{
		g_devConsole->PrintError( "GameConfig.xml's playerActorName was not loaded from EntityTypes.xml" );
		return;
	}

	m_player = new Actor( *playerDef );
	m_player->SetAsPlayer();

	g_devConsole->PrintString( "Entity Types Loaded", Rgba8::GREEN );
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
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				ChangeGameState( eGameState::PAUSED );
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
			{
				ChangeMap( m_curMapName );
			}
		}
		case eGameState::PAUSED:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_eventSystem->FireEvent( "Quit" );
			}
		}
		break;

	}
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMap( const std::string& mapName )
{
	m_world->ChangeMap( mapName, m_player );
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
void Game::WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	// No entity specified, just load the new map and set camera position and orientation
	if ( entityToWarp == nullptr )
	{
		if ( destMapName != "" )
		{
			m_world->ChangeMap( destMapName, m_player );
		}
		
		return;
	}

	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
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
			m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true, .1f );
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
					g_audioSystem->PlaySound( unpause, false, .1f );

					g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, 1.f );
				}
				break;

				case eGameState::ATTRACT:
				{
					g_audioSystem->StopSound( m_attractMusicID );

					SoundID gameplayMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/GameplayMusic.mp3" );
					m_gameplayMusicID = g_audioSystem->PlaySound( gameplayMusic, true, .1f );
					
					m_curMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMapName );
					g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMapName.c_str() ) );
					m_world->ChangeMap( m_curMapName, m_player );
				}
				break;
			}
		}
		break;

		case eGameState::PAUSED:
		{
			g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, .5f );

			SoundID pause = g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
			g_audioSystem->PlaySound( pause, false, .1f );

		}
		break;

		case eGameState::VICTORY:
		{
			//m_curVictoryScreenSeconds = 0.f;

			g_audioSystem->StopSound( m_gameplayMusicID );

			SoundID victoryMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
			m_victoryMusicID = g_audioSystem->PlaySound( victoryMusic, true, .1f );
		}
		break;
	}

	m_gameState = newGameState;
}
