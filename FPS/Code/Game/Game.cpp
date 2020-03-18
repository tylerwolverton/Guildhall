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
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"

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
	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );

	m_worldCamera = new Camera();
	m_worldCamera->SetColorTarget( nullptr );
	Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
	m_worldCamera->SetDepthStencilTarget( depthTexture );

	m_worldCamera->SetOutputSize( Vec2( 16.f, 9.f ) );
	//m_worldCamera->SetProjectionOrthographic( 9.f, 0.f, -100.f );
	m_worldCamera->SetProjectionPerspective( 60.f, -.1f, -100.f );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );
	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	EnableDebugRendering();

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );

	std::vector<Vertex_PCU> vertices;
	AppendVertsForCubeMesh( vertices, Vec3::ZERO, 2.f, Rgba8::WHITE );
	
	std::vector<uint> indices;
	AppendIndicesForCubeMesh( indices );	

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );
	m_cubeMeshTransform.SetPosition( Vec3( 1.f, .5f, -12.f ) );

	vertices.clear();
	indices.clear();
	AppendVertsForPlaneMesh( vertices, Vec3::ZERO, Vec2( 7.f, 5.f) , 64, 32, Rgba8::WHITE );
	AppendIndicesForPlaneMesh( indices, 64, 32 );

	m_planeMesh = new GPUMesh( g_renderer, vertices, indices );
	m_planeMeshTransform.SetPosition( Vec3( -8.f, .5f, -6.f ) );

	// Create Spheres
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForSphereMesh( vertices, indices, Vec3::ZERO, 1.f, 64, 32, Rgba8::WHITE );

	m_sphereMesh = new GPUMesh( g_renderer, vertices, indices );
	Transform centerTransform;
	centerTransform.SetPosition( Vec3( 6.f, 4.f, -8.f ) );
	m_sphereMeshTransforms.push_back( centerTransform );

	// Create sphere ring
	m_sphereMeshTransforms.reserve( NUM_SPHERES );
	for ( int sphereNum = 0; sphereNum < NUM_SPHERES; ++sphereNum )
	{
		float currentDegrees = (float)sphereNum * DEGREES_PER_SPHERE;
		Transform sphereTransform;
		sphereTransform.SetPosition( Vec3( CosDegrees( currentDegrees ), SinDegrees( currentDegrees ), 0.f ) * SPHERE_RING_RADIUS );

		m_sphereMeshTransforms.push_back( sphereTransform );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );
	//g_inputSystem->SetCursorMode( CURSOR_ABSOLUTE );

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_cubeMesh );
	PTR_SAFE_DELETE( m_planeMesh );
	PTR_SAFE_DELETE( m_sphereMesh );
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_gameClock );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
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
	if ( !g_devConsole->IsOpen() ) 
	{
		UpdateFromKeyboard();
	}

	UpdateCameras();
	
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, Rgba8::BLACK );

	m_cubeMeshTransform.SetRotationFromPitchRollYawDegrees( 0.f, 0.f,  (float)( GetCurrentTimeSeconds() * 20.f ) );

	for ( int transformIdx = 0; transformIdx < (int)m_sphereMeshTransforms.size(); ++transformIdx )
	{
		float currentDegrees = (float)transformIdx * DEGREES_PER_SPHERE + (float)GetCurrentTimeSeconds() * 5.f;
		Transform& sphereTransform = m_sphereMeshTransforms[transformIdx];
		// Don't move initial sphere
		if ( transformIdx != 0 )
		{
			Vec3 position = sphereTransform.GetPosition();
			sphereTransform.SetPosition( Vec3( CosDegrees( currentDegrees ), SinDegrees( currentDegrees ), 0.f ) * SPHERE_RING_RADIUS );
		}

		sphereTransform.SetRotationFromPitchRollYawDegrees( 0.f, 0.f, (float)( GetCurrentTimeSeconds() * ( 10.f * transformIdx + 5.f ) ) );
	
	}
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	Texture* texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/firewatch_150305_06.png" );
	g_renderer->BindTexture( texture );
	g_renderer->BindShader( "Data/Shaders/Default.hlsl" );
	
	DrawAABB2WithDepth( g_renderer, AABB2( -.5f, -.5f, .5f, .5f ), -10.f, Rgba8::WHITE );
	
	Mat44 model = m_cubeMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model );
	g_renderer->DrawMesh( m_cubeMesh );
		
	model = m_planeMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model );
	g_renderer->DrawMesh( m_planeMesh );

	for ( int transformIdx = 0; transformIdx < (int)m_sphereMeshTransforms.size(); ++transformIdx )
	{
		model = m_sphereMeshTransforms[transformIdx].GetAsMatrix();
		g_renderer->SetModelMatrix( model );
		g_renderer->DrawMesh( m_sphereMesh );
	}

	g_renderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

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
	if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
	{
		g_renderer->CycleCullMode();
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
	{
		g_renderer->CycleFillMode();
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F6 ) )
	{
		g_renderer->CycleWindOrder();
	}
	if ( g_inputSystem->WasKeyJustPressed( 'Q' ) )
	{
		DebugAddWorldPoint( m_worldCamera->GetTransform().GetPosition(), .1f, Rgba8::BLUE, Rgba8::RED, 3.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadNewMap( const std::string& mapName )
{
	PTR_SAFE_DELETE( m_world );

	m_world = new World();
	m_world->BuildNewMap( mapName );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{

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

	m_debugInfoTextBox->SetText( textLines[0], color );

	for ( int textLineIndex = 1; textLineIndex < (int)textLines.size(); ++textLineIndex )
	{
		m_debugInfoTextBox->AddLineOFText( textLines[ textLineIndex ], color );
	}
}


//-----------------------------------------------------------------------------------------------
bool Game::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "multiplier", 1.f );

	return false;
}
