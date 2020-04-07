#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
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
	m_worldCamera->SetProjectionPerspective( 60.f, -.1f, -100.f );
	
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );
	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	EnableDebugRendering();

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );

	// Create Sphere and Cube
	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForCubeMesh( vertices, indices, Vec3::ZERO, 2.f, Rgba8::WHITE );
	
	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );
	m_cubeMeshTransform.SetPosition( Vec3( -5.f, 0.f, -6.f ) );
	
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForQuad( vertices, indices, AABB2( -1.f, -1.f, 1.f, 1.f ), Rgba8::WHITE );
	m_quadMesh = new GPUMesh( g_renderer, vertices, indices );
	m_quadMeshTransform.SetPosition( Vec3( 0.f, 0.f, -6.f ) );

	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForSphereMesh( vertices, indices, Vec3::ZERO, 1.f, 64, 32, Rgba8::WHITE );

	m_sphereMesh = new GPUMesh( g_renderer, vertices, indices );
	Transform centerTransform;
	centerTransform.SetPosition( Vec3( 5.f, 0.f, -6.f ) );
	m_sphereMeshTransform = centerTransform;

	m_pointLight.color = Vec3::ONE;

	// Init shaders
	m_shaderPaths.push_back( "Data/Shaders/Lit.hlsl" );
	m_shaderNames.push_back( "Lit" );

	m_shaderPaths.push_back( "Data/Shaders/Default.hlsl" );
	m_shaderNames.push_back( "Default" );

	m_shaderPaths.push_back( "Data/Shaders/Normals.hlsl" );
	m_shaderNames.push_back( "Normals" );
	m_shaderPaths.push_back( "Data/Shaders/Tangents.hlsl" );
	m_shaderNames.push_back( "Tangents" );
	m_shaderPaths.push_back( "Data/Shaders/Bitangents.hlsl" );
	m_shaderNames.push_back( "Bitangents" );
	m_shaderPaths.push_back( "Data/Shaders/SurfaceNormals.hlsl" );
	m_shaderNames.push_back( "Surface Normals" );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_quadMesh );
	PTR_SAFE_DELETE( m_cubeMesh );
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
	m_cubeMeshTransform.SetRotationFromPitchRollYawDegrees( (float)( GetCurrentTimeSeconds() * 6.f ), 0.f, (float)( GetCurrentTimeSeconds() * 20.f ) );
	m_sphereMeshTransform.SetRotationFromPitchRollYawDegrees( (float)( GetCurrentTimeSeconds() * 20.f ), 0.f, (float)( GetCurrentTimeSeconds() * 30.f ) );

	switch ( m_lightMode )
	{
		case eLightMode::STATIONARY:
		{
			DebugAddWorldPoint( m_pointLight.position, Rgba8::GREEN );
		} break;
		case eLightMode::FOLLOW_CAMERA:
		{
			m_pointLight.position = m_worldCamera->GetTransform().GetPosition();
		} break;
		case eLightMode::LOOP:
		{
			DebugAddWorldPoint( m_pointLight.position, Rgba8::GREEN );
		}
	}

	PrintHotkeys();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->BindDiffuseTexture( nullptr );
	g_renderer->BindNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/brick_normal.png" ) );
	g_renderer->BindShader( m_shaderPaths[m_currentShaderIdx].c_str() );
	
	g_renderer->SetAmbientLight( m_ambientColor, m_ambientIntensity );
	g_renderer->EnableLight( 0, m_pointLight );

	g_renderer->SetMaterialData( m_specularFactor, m_specularPower );

	Mat44 model = m_cubeMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model, Rgba8::YELLOW );
	g_renderer->DrawMesh( m_cubeMesh );

	model = m_quadMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model, Rgba8::WHITE );
	g_renderer->DrawMesh( m_quadMesh );

	model = m_sphereMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model, Rgba8::GREEN );
	g_renderer->DrawMesh( m_sphereMesh );
   
	g_renderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::PrintHotkeys()
{
	DebugAddScreenText( Vec4( 0.f, .95f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F5 - Move light to origin" );
	DebugAddScreenText( Vec4( 0.f, .90f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F6 - Move light to camera" );
	DebugAddScreenText( Vec4( 0.f, .85f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F7 - Make light follow camera" );
	DebugAddScreenText( Vec4( 0.f, .80f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F8 - Make light loop" );
	DebugAddScreenTextf( Vec4( 0.f, .75f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "9,0 - Change intensity of ambient light : %.2f", m_ambientIntensity );
	DebugAddScreenTextf( Vec4( 0.f, .70f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "-,+ - Change intensity of point light : %.2f", m_pointLight.intensity );
	DebugAddScreenTextf( Vec4( 0.f, .65f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "[,] - Change specular factor : %.2f", m_specularFactor );
	DebugAddScreenTextf( Vec4( 0.f, .60f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, ";,' - Change specular power : %.2f", m_specularPower );
	DebugAddScreenTextf( Vec4( 0.f, .55f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "<,> - Change shader : %s", m_shaderNames[m_currentShaderIdx].c_str() );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeShader( int nextShaderIdx )
{
	if ( nextShaderIdx > (int)m_shaderPaths.size() - 1 )
	{
		nextShaderIdx = 0;
	}
	else if ( nextShaderIdx < 0 )
	{
		nextShaderIdx = (int)m_shaderPaths.size() - 1;
	}

	m_currentShaderIdx = nextShaderIdx;
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
	if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
	{
		m_pointLight.position = Vec3::ZERO;
		m_lightMode = eLightMode::STATIONARY;
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F6 ) )
	{
		m_pointLight.position = m_worldCamera->GetTransform().GetPosition();
		m_lightMode = eLightMode::STATIONARY;
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F7 ) )
	{
		m_lightMode = eLightMode::FOLLOW_CAMERA;
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F8 ) )
	{
		m_lightMode = eLightMode::LOOP;
	}

	// Debug Commands
	if ( g_inputSystem->IsKeyPressed( 'Q' ) )
	{
		DebugAddWorldPoint( m_worldCamera->GetTransform().GetPosition(), .01f, Rgba8::GREEN, Rgba8::RED, 10.f, DEBUG_RENDER_XRAY );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'O' ) )
	{
		DebugAddWorldLine( m_worldCamera->GetTransform().GetPosition(), Rgba8::RED, Rgba8::GREEN, 
						   m_cubeMeshTransform.GetPosition(), Rgba8::WHITE, Rgba8::BLACK, 
						   10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'E' ) )
	{
		DebugAddWorldArrow( m_worldCamera->GetTransform().GetPosition(), Rgba8::RED, Rgba8::GREEN,
							m_cubeMeshTransform.GetPosition(), Rgba8::WHITE, Rgba8::BLACK,
							10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'R' ) )
	{
		AABB3 box( m_worldCamera->GetTransform().GetPosition(), m_worldCamera->GetTransform().GetPosition() + Vec3::ONE );
		DebugAddWorldWireBounds( box, Rgba8::YELLOW, 10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'Y' ) )
	{
		DebugAddWorldWireSphere( m_worldCamera->GetTransform().GetPosition(), 2.f, Rgba8::WHITE, Rgba8::BLUE, 10.f, DEBUG_RENDER_ALWAYS );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'T' ) )
	{
		Mat44 textView = m_worldCamera->GetViewMatrix();
		InvertOrthoNormalMatrix( textView );
		DebugAddWorldText( textView, Vec2::ZERO, Rgba8::BLUE, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, "Zero!" );
		DebugAddWorldText( textView, Vec2( .5f, .5f ), Rgba8::GREEN, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, "Mid!" );
		DebugAddWorldTextf( textView, Vec2::ONE, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, "One!", 12 );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'B' ) )
	{
		DebugAddWorldBillboardText( m_worldCamera->GetTransform().GetPosition() - m_worldCamera->GetTransform().GetAsMatrix().GetKBasis3D() * 5.f, Vec2::ONE, Rgba8::GREEN, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_XRAY, "Mid!" );
		DebugAddWorldBillboardTextf( m_worldCamera->GetTransform().GetPosition() - m_worldCamera->GetTransform().GetAsMatrix().GetKBasis3D() * 5.f, Vec2::ZERO, Rgba8::GREEN, 35.f, eDebugRenderMode::DEBUG_RENDER_XRAY, "%d!", 15 );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'U' ) )
	{
		Vec3 p0 = m_worldCamera->GetTransform().GetPosition();
		Vec3 p1 = p0 + Vec3( 10.f, 0.f, 0.f );
		Vec3 p2 = p0 + Vec3( 0.f, 10.f, 0.f );
		Vec3 p3 = p0 + Vec3( 10.f, 10.f, 0.f );
		DebugAddWorldQuad( p0, p1, p2, p3, AABB2::ONE_BY_ONE, Rgba8::GREEN, Rgba8::RED, 20.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'I' ) )
	{
		DebugAddWorldBasis( m_worldCamera->GetTransform().GetAsMatrix(), Rgba8::WHITE, Rgba8::BLACK, 10.f );
	}

	if ( g_inputSystem->WasKeyJustPressed( '1' ) )
	{
		DebugAddScreenPoint( Vec2( 1920.f, 1080.f ) * .5f, 4.f, Rgba8::GREEN, Rgba8::RED, 5.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '2' ) )
	{
		DebugAddScreenLine( Vec2::ZERO, Rgba8::BLUE, Vec2( 1920.f, 1080.f ) * .5f, Rgba8::WHITE, Rgba8::BLACK, Rgba8::YELLOW, 3.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '3' ) )
	{
		DebugAddScreenArrow( Vec2(250.f, 300.f), Rgba8::BLUE, Vec2( 1000.f, 12.f ), Rgba8::GREEN, Rgba8::RED, Rgba8::YELLOW, 3.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '4' ) )
	{
		DebugAddScreenQuad( AABB2( Vec2( 250.f, 12.f ), Vec2( 1000.f, 120.f ) ), Rgba8::GREEN, Rgba8::RED, 10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '5' ) )
	{
		Texture* texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/firewatch_150305_06.png" );
		DebugAddScreenTexturedQuad( AABB2( Vec2( 10.f, 10.f ), Vec2( 500.f, 500.f ) ), texture, Rgba8::WHITE, 10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '6' ) )
	{
		DebugAddScreenTextf( Vec4( .5f, .75f, 10.f, -10.f ), Vec2( .5f, .5f ), 10.f, Rgba8::WHITE, 10.f, "Here is some text %d", 13 );
	}

	if ( g_inputSystem->IsKeyPressed( '9' ) )
	{
		m_ambientIntensity -= .5f * deltaSeconds;
		m_ambientIntensity = ClampZeroToOne( m_ambientIntensity );
	}
	if ( g_inputSystem->IsKeyPressed( '0' ) )
	{
		m_ambientIntensity += .5f * deltaSeconds;
		m_ambientIntensity = ClampZeroToOne( m_ambientIntensity );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_MINUS ) )
	{
		m_pointLight.intensity -= .5f * deltaSeconds;
		m_pointLight.intensity = ClampZeroToOne( m_pointLight.intensity );
	}
	if ( g_inputSystem->IsKeyPressed( KEY_PLUS ) )
	{
		m_pointLight.intensity += .5f * deltaSeconds;
		m_pointLight.intensity = ClampZeroToOne( m_pointLight.intensity );
	}
	if ( g_inputSystem->IsKeyPressed( KEY_LEFT_BRACKET ) )
	{
		m_specularFactor -= .5f * deltaSeconds;
		m_specularFactor = ClampZeroToOne( m_specularFactor );
	}
	if ( g_inputSystem->IsKeyPressed( KEY_RIGHT_BRACKET ) )
	{
		m_specularFactor += .5f * deltaSeconds;
		m_specularFactor = ClampZeroToOne( m_specularFactor );
	}
	if ( g_inputSystem->IsKeyPressed( KEY_SEMICOLON ) )
	{
		m_specularPower -= 2.f * deltaSeconds;
		m_specularPower = ClampMin( m_specularPower, 1.f );
	}
	if ( g_inputSystem->IsKeyPressed( KEY_QUOTE ) )
	{
		m_specularPower += 2.f * deltaSeconds;
		m_specularPower = ClampMin( m_specularPower, 1.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_COMMA ) )
	{
		ChangeShader( m_currentShaderIdx - 1 );
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_PERIOD ) )
	{
		ChangeShader( m_currentShaderIdx + 1 );
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
	Rgba8 backgroundColor( 10, 10, 10, 255 );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, backgroundColor );
}


//-----------------------------------------------------------------------------------------------
void Game::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Mat44 model = m_worldCamera->GetTransform().GetAsMatrix();
	Vec3 absoluteTranslation = model.TransformVector3D( relativeTranslation );

	m_worldCamera->Translate( absoluteTranslation );
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
