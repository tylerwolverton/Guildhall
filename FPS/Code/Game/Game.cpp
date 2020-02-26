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
#include "Engine/Renderer/GPUMesh.hpp"
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
	m_worldCamera->SetColorTarget( nullptr );

	m_rng = new RandomNumberGenerator();

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );

	m_mesh = new GPUMesh( g_renderer );
	m_meshTransform.SetPosition( Vec3( 1.f, .5f, -12.f ) );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;
	
	g_renderer->AppendVertsForCubeMesh( vertices, Vec3::ZERO, 2.f, Rgba8::WHITE );
	g_renderer->AppendIndicesForCubeMesh( indices );

	// Update buffers
	m_mesh->UpdateVertices( vertices.size(), &vertices[0] );
	m_mesh->UpdateIndices( indices.size(), &indices[0] );
	/*mesh->AddVertices( 24, cubeVerts );
	mesh->AddIndices( 36, indices );*/
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
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
	if ( !g_devConsole->IsOpen() ) 
	{
		UpdateFromKeyboard( deltaSeconds );
	}

	UpdateCameras( deltaSeconds );
	
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );

	m_meshTransform.SetRotationFromPitchYawRollDegrees( 0.f, GetCurrentTimeSeconds() * 20.f, 0.f );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera(*m_worldCamera );
	
	/*Texture* texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/firewatch_150305_06.png" );
	g_renderer->BindTexture( texture );*/
	g_renderer->BindShader( "Data/Shaders/Default.hlsl" );
	
	//g_renderer->DrawMesh( mesh );
	g_renderer->DrawAABB2WithDepth( AABB2( -.5f, -.5f, .5f, .5f ), -10.f, Rgba8::WHITE );
	
	Mat44 model = m_meshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model );
	g_renderer->DrawMesh( m_mesh );

	/*std::vector<Vertex_PCU> vertices;
	g_renderer->AppendVertsForAABB2D( vertices, AABB2( -.5f, -.5f, .5f, .5f ), Rgba8::WHITE );

	std::vector<uint> indices = { 0, 1, 2, 3, 4, 5 };

	g_renderer->DrawIndexed( vertices.size(), &vertices[0], indices );*/


	
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
	Vec3 cameraRotation;

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
		cameraTranslation.z -= 10.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		cameraTranslation.z += 10.f;
	}
	
	if ( g_inputSystem->IsKeyPressed( KEY_UPARROW ) )
	{
		cameraTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_DOWNARROW ) )
	{
		cameraTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_RIGHTARROW ) )
	{
		cameraRotation.y += 30.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_LEFTARROW ) )
	{
		cameraRotation.y -= 30.f;
	}

	m_worldCamera->Translate( cameraTranslation * deltaSeconds );
	Transform transform = m_worldCamera->GetTransform();
	m_worldCamera->SetPitchRollYawRotation( 0.f, transform.m_rotation.y + ( cameraRotation.y * deltaSeconds ), 0.f );

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
void Game::UpdateMousePositions( float deltaSeconds )
{
	UpdateMouseWorldPosition( deltaSeconds );
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

	//m_worldCamera->SetOrthoView( Vec2( -1.f, -1.f ), Vec2( 1.f, 1.f ) );
	m_worldCamera->SetOutputSize( Vec2( 2.f, 2.f ) );
	//m_worldCamera->SetProjectionOrthographic( Vec2( 2.f, 2.f ), 0.f, 1.f );
	m_worldCamera->SetProjectionPerspective( 60.f, -.1f, -100.f );
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
