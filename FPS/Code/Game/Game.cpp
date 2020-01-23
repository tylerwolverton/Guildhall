#include "Game.hpp"
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
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"


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
	m_uiCamera = new Camera();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	LoadAssets();

	m_world = new World();

	m_curMap = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMap );
	g_devConsole->PrintString( Rgba8::WHITE, Stringf( "Loading starting map: %s", m_curMap.c_str() ) );
	m_world->BuildNewMap( m_curMap );

	LogMapDebugCommands();
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	TileDefinition::s_definitions.clear();

	// Clean up global sprite sheets
	delete g_tileSpriteSheet;
	g_tileSpriteSheet = nullptr;

	delete g_characterSpriteSheet;
	g_characterSpriteSheet = nullptr;

	delete g_portraitSpriteSheet;
	g_portraitSpriteSheet = nullptr;

	// Clean up member variables
	delete m_world;
	m_world = nullptr;

	delete m_rng;
	m_rng = nullptr;
	
	delete m_debugInfoTextBox;
	m_debugInfoTextBox = nullptr;
	
	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;
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
	g_devConsole->PrintString(Rgba8::WHITE, "Map Generation Debug Commands" );
	g_devConsole->PrintString(Rgba8::WHITE, "F4 - View entire map" );
	g_devConsole->PrintString(Rgba8::WHITE, "F5 - Reload current map" );
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
	UpdateFromKeyboard( deltaSeconds );

	// Modify deltaSeconds based on game state
	if ( m_isPaused )
	{
		deltaSeconds = 0.f;
	}
	if ( m_isSlowMo )
	{
		deltaSeconds *= .1f;
	}
	if ( m_isFastMo )
	{
		deltaSeconds *= 4.f;
	}
	
	m_world->Update( deltaSeconds );
	UpdateCameras( deltaSeconds );
	UpdateMousePositions( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_renderer->ClearScreen(Rgba8(0, 0, 0));

	g_renderer->BeginCamera(*m_worldCamera );

	m_world->Render();
	if ( m_isDebugRendering )
	{
		m_world->DebugRender();
	}
	
	g_renderer->EndCamera( *m_worldCamera );

	// Render UI with a new camera
	g_renderer->BeginCamera( *m_uiCamera );

	g_devConsole->Render( *g_renderer, *m_uiCamera, 20 );

	if ( m_isDebugRendering )
	{
		m_debugInfoTextBox->Render( m_mouseUIPosition );
	}
	
	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( Rgba8::WHITE, "Loading Assets..." );
	g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );

	// TODO: Check for nullptrs when loading textures
	g_tileSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_32x32.png" )), IntVec2( 32, 32 ) );
	g_characterSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaCharacters_12x53.png" )), IntVec2( 12, 53 ) );
	g_portraitSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaPortraits_8x8.png" )), IntVec2( 8, 8 ) );

	LoadTilesFromXml();
	LoadMapsFromXml();
	LoadActorsFromXml();

	g_devConsole->PrintString( Rgba8::GREEN, "Assets Loaded" );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTilesFromXml()
{
	g_devConsole->PrintString( Rgba8::WHITE, "Loading Tiles..." );

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

	g_devConsole->PrintString( Rgba8::GREEN, "Tiles Loaded" );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadMapsFromXml()
{
	g_devConsole->PrintString( Rgba8::WHITE, "Loading Maps..." );

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

	g_devConsole->PrintString( Rgba8::GREEN, "Maps Loaded" );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadActorsFromXml()
{
	g_devConsole->PrintString( Rgba8::WHITE, "Loading Actors..." );

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

	g_devConsole->PrintString( Rgba8::GREEN, "Actors Loaded" );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	m_isSlowMo = g_inputSystem->IsKeyPressed('T');
	m_isFastMo = g_inputSystem->IsKeyPressed('Y');

	if ( g_inputSystem->WasKeyJustPressed('P') )
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

	if ( g_inputSystem->WasKeyJustPressed( KEY_TILDE ) )
	{
		g_devConsole->ToggleOpenFull();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadNewMap( const std::string& mapName )
{
	delete m_world;
	m_world = nullptr;
	m_world = new World();
	m_world->BuildNewMap( mapName );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMousePositions( float deltaSeconds )
{
	UpdateMouseWorldPosition( deltaSeconds );
	UpdateMouseUIPosition( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseWorldPosition( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	Vec2 worldWindowDimensions = m_worldCamera->GetOrthoTopRight() - m_worldCamera->GetOrthoBottomLeft();
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos() * worldWindowDimensions;
	m_mouseWorldPosition += m_worldCamera->GetOrthoBottomLeft();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseUIPosition( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	Vec2 uiWindowDimensions = m_uiCamera->GetOrthoTopRight() - m_uiCamera->GetOrthoBottomLeft();
	m_mouseUIPosition = g_inputSystem->GetNormalizedMouseClientPos() * uiWindowDimensions;
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
void Game::PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines )
{
	if ( (int)textLines.size() == 0 )
	{
		return;
	}

	m_debugInfoTextBox->SetText( color, textLines[0] );

	for ( int textLineIndex = 1; textLineIndex < (int)textLines.size(); ++textLineIndex )
	{
		m_debugInfoTextBox->AddLineOFText( color, textLines[ textLineIndex ] );
	}
}
