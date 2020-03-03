#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
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
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"


static float s_mouseSensitivityMultiplier = 1.f;


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
	g_eventSystem->RegisterEvent( "SetMouseSensitivity", "Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );

	g_inputSystem->SetCursorMode( CURSOR_RELATIVE );

	m_worldCamera = new Camera();
	m_worldCamera->SetColorTarget( nullptr );
	Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
	m_worldCamera->SetDepthStencilTarget( depthTexture );

	m_worldCamera->SetOutputSize( Vec2( 16.f, 9.f ) );
	//m_worldCamera->SetProjectionOrthographic( 9.f, 0.f, -100.f );
	m_worldCamera->SetProjectionPerspective( 60.f, -.1f, -100.f );

	m_rng = new RandomNumberGenerator();

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );

	std::vector<Vertex_PCU> vertices;
	AppendVertsForCubeMesh( vertices, Vec3::ZERO, 2.f, Rgba8::WHITE );
	
	std::vector<uint> indices;
	AppendIndicesForCubeMesh( indices );	

	m_mesh = new GPUMesh( g_renderer, vertices, indices );
	m_meshTransform.SetPosition( Vec3( 1.f, .5f, -12.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->SetCursorMode( CURSOR_ABSOLUTE );

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	delete m_mesh;
	m_mesh = nullptr;

	delete m_world;
	m_world = nullptr;

	delete m_rng;
	m_rng = nullptr;
	
	delete m_debugInfoTextBox;
	m_debugInfoTextBox = nullptr;
	
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
void Game::Update( float deltaSeconds )
{
	if ( !g_devConsole->IsOpen() ) 
	{
		UpdateFromKeyboard( deltaSeconds );
	}

	UpdateCameras( deltaSeconds );
	
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, Rgba8::BLACK );

	m_meshTransform.SetRotationFromPitchRollYawDegrees( 0.f, 0.f,  (float)( GetCurrentTimeSeconds() * 20.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera(*m_worldCamera );

	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	Texture* texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/firewatch_150305_06.png" );
	g_renderer->BindTexture( texture );
	g_renderer->BindShader( "Data/Shaders/Default.hlsl" );
	
	DrawAABB2WithDepth( g_renderer, AABB2( -.5f, -.5f, .5f, .5f ), -10.f, Rgba8::WHITE );
	
	Mat44 model = m_meshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model );
	g_renderer->DrawMesh( m_mesh );
		
	g_renderer->EndCamera( *m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	Vec3 cameraTranslation;

	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		cameraTranslation.x += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		cameraTranslation.x -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		cameraTranslation.z -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		cameraTranslation.z += 1.f;
	}
	
	if ( g_inputSystem->IsKeyPressed( 'C' ) )
	{
		cameraTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SPACEBAR ) )
	{
		cameraTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
	{
		cameraTranslation *= 10.f;
	}

	// Rotation
	Vec2 mousePosition = g_inputSystem->GetMouseDeltaPosition();
	float yaw = -mousePosition.x * s_mouseSensitivityMultiplier;
	float pitch = -mousePosition.y * s_mouseSensitivityMultiplier;

	Transform transform = m_worldCamera->GetTransform();
	m_worldCamera->SetPitchRollYawRotation( transform.m_rotation.x + pitch * deltaSeconds,
											0.f,
											transform.m_rotation.z + yaw * deltaSeconds );

	// Translation
	TranslateCameraFPS( cameraTranslation * deltaSeconds );

	if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
	{
		g_renderer->CycleSampler();
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
	{
		g_renderer->CycleBlendMode();
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
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Mat44 model = m_worldCamera->GetTransform().GetAsMatrix();
	Vec3 absoluteTranslation = model.TransformVector3D( relativeTranslation );

	m_worldCamera->Translate( absoluteTranslation );
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


//-----------------------------------------------------------------------------------------------
bool Game::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "SetMouseSensitivity", 1.f );

	return false;
}
