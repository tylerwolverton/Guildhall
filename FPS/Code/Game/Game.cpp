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
#include "Engine/Core/Time.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EventSystem.hpp"
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

	m_rng = new RandomNumberGenerator();
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
	UNUSED( deltaSeconds );
	//UpdateFromKeyboard( deltaSeconds );

	float seconds = (float)GetCurrentTimeSeconds();
	float green = RangeMapFloat( -1.f, 1.f, 0.f, 255.f, SinDegrees( seconds * 40.f ) );
	float blue = RangeMapFloat( -1.0f, 1.0f, 0.0f, 255.0f, CosDegrees( seconds * 30.f ) );
	Rgba8 clearColor = Rgba8( 0, (unsigned char)green, (unsigned char)blue, 255);

	// clear to a different frame each time
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, clearColor );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera(*m_worldCamera );

	g_renderer->Draw( 3 );

	g_renderer->EndCamera( *m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( Rgba8::WHITE, "Loading Assets..." );

	g_devConsole->PrintString( Rgba8::GREEN, "Assets Loaded" );
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
