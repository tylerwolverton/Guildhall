#include "Game/PlayerClient.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/GameEvents.hpp"
#include "Game/Server.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
static float s_mouseSensitivityMultiplier = 1.f;
static Vec3 s_ambientLightColor = Vec3( 1.f, 1.f, 1.f );


//-----------------------------------------------------------------------------------------------
void PlayerClient::Startup()
{
	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetAmbientLightColor );

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );
	// Probably do something to hook up to RemoteServer if in MULTIPLAYER_CLIENT mode

	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT - 1; ++frameNum )
	{
		m_fpsHistory[frameNum] = 60.f;
	}
	
	m_gameClock = new Clock();

	g_renderer->Setup( m_gameClock );

	EnableDebugRendering();

	InitializeCameras();
	
	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );
	BuildUIHud();

	std::vector<const ClientRequest*> clientRequests;
	clientRequests.push_back( new CreateEntityRequest( m_clientId, -1, eEntityType::PLAYER, Vec2( 5.5f, 4.f ), 0.f ) );

	/*m_player = g_game->CreateEntityInCurrentMap( eEntityType::PLAYER, Vec2( 5.5f, 4.f ), 0.f );
	m_player->Possess();*/

	g_server->ReceiveClientRequests( clientRequests );

	PTR_VECTOR_SAFE_DELETE( clientRequests );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::BeginFrame()
{
	std::vector<const ClientRequest*> clientRequests = ProcessInputAndConvertToClientRequests();
	g_server->ReceiveClientRequests( clientRequests );	

	PTR_VECTOR_SAFE_DELETE( clientRequests );
}


//-----------------------------------------------------------------------------------------------
std::vector<const ClientRequest*> PlayerClient::ProcessInputAndConvertToClientRequests()
{
	std::vector<const ClientRequest*> requests;

	if ( g_devConsole->IsOpen() )
	{
		return requests;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
	{
		g_raytraceFollowCamera = !g_raytraceFollowCamera;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
	{
		if ( m_player == nullptr )
		{
			//requests.push_back( new PossessEntityRequest( m_player, m_worldCamera->GetTransform() ) );
			//PossesNearestEntity();
		}
		else
		{
			//requests.push_back( new UnPossessEntityRequest( m_player ) );
			//m_player->Unpossess();
			//m_player = nullptr;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
	{
		g_renderer->ReloadShaders();
	}

	Vec3 movementTranslation;
	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		movementTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		movementTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		movementTranslation.x += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		movementTranslation.x -= 1.f;
	}

	if ( m_player == nullptr )
	{
		if ( g_inputSystem->IsKeyPressed( 'E' ) )
		{
			movementTranslation.z += 1.f;
		}

		if ( g_inputSystem->IsKeyPressed( 'Q' ) )
		{
			movementTranslation.z -= 1.f;
		}

		if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
		{
			movementTranslation *= 10.f;
		}
	}

	// Rotation
	Vec2 mousePosition = g_inputSystem->GetMouseDeltaPosition();
	float yawDegrees = -mousePosition.x * s_mouseSensitivityMultiplier;
	float pitchDegrees = mousePosition.y * s_mouseSensitivityMultiplier;
	yawDegrees *= .009f;
	pitchDegrees *= .009f;

	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	// An entity is possessed
	if ( m_player != nullptr )
	{
		Vec2 forwardVec = m_player->GetForwardVector();
		Vec2 rightVec = forwardVec.GetRotatedMinus90Degrees();

		Vec2 translationXY( movementTranslation.x * forwardVec
							+ movementTranslation.y * rightVec );

		translationXY *= m_player->GetWalkSpeed();

		requests.push_back( new UpdateEntityRequest( m_clientId, m_player->GetId(), translationXY * deltaSeconds, yawDegrees ) );
	}
	// No entity possessed, move the camera directly
	else
	{
		Transform transform = m_worldCamera->GetTransform();
		m_worldCamera->RotateYawPitchRoll( yawDegrees, pitchDegrees, 0.f );

		// Translation
		TranslateCameraFPS( movementTranslation * deltaSeconds );
	}

	return requests;
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::InitializeCameras()
{
	m_worldCamera = new Camera();
	Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
	m_worldCamera->SetDepthStencilTarget( depthTexture );

	m_worldCamera->SetOutputSize( Vec2( 16.f, 9.f ) );
	m_worldCamera->SetProjectionPerspective( 60.f, -.05f, -100.f );
	m_worldCamera->Translate( Vec3( 0.f, 0.f, .5f ) );

	Rgba8 backgroundColor( 10, 10, 10, 255 );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, backgroundColor );

	Vec2 windowDimensions = g_window->GetDimensions();

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetType( eCameraType::UI );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::BuildUIHud()
{
	Texture* hudBaseTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Hud_Base.png" );

	UIAlignedPositionData panelPosData;
	panelPosData.fractionOfParentDimensions = Vec2( 1.f, .13f );
	panelPosData.alignmentWithinParentElement = ALIGN_BOTTOM_CENTER;
	m_hudUIPanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPosData, hudBaseTexture );

	panelPosData.fractionOfParentDimensions = Vec2( 1.f, .87f );
	panelPosData.alignmentWithinParentElement = ALIGN_TOP_CENTER;
	m_worldUIPanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPosData );

	// Add gun sprite
	SpriteSheet* gunSprite = SpriteSheet::GetSpriteSheetByName( "ViewModels" );
	Texture* texture = const_cast<Texture*>( &gunSprite->GetTexture() );
	Vec2 uvsAtMins, uvsAtMaxs;
	gunSprite->GetSpriteUVs( uvsAtMins, uvsAtMaxs, IntVec2::ZERO );

	UIAlignedPositionData gunPanelPosData;
	gunPanelPosData.fractionOfParentDimensions = Vec2( .5f, 1.f );
	gunPanelPosData.alignmentWithinParentElement = ALIGN_BOTTOM_CENTER;
	m_worldUIPanel->AddChildPanel( gunPanelPosData, texture, Rgba8::WHITE, uvsAtMins, uvsAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Shutdown()
{
	g_inputSystem->PopMouseOptions();

	m_uiSystem->Shutdown();

	PTR_SAFE_DELETE( m_gameClock );
	PTR_SAFE_DELETE( m_worldCamera );
	PTR_SAFE_DELETE( m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Update()
{
	UpdateFramesPerSecond();

	m_uiSystem->Update();

	UpdateCameraTransformToMatchPlayer( g_inputSystem->GetMouseDeltaPosition() );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::Render( const World* gameWorld ) const
{
	g_devConsole->Render();

	RenderFPSCounter();

	Texture* backbuffer = g_renderer->GetBackBuffer();
	Texture* colorTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );

	m_worldCamera->SetColorTarget( 0, colorTarget );

	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	g_renderer->SetGamma( m_gamma );

	if ( gameWorld != nullptr )
	{
		gameWorld->Render();
	}

	g_renderer->EndCamera( *m_worldCamera );

	// Copy rendered data to backbuffer and set on camera
	g_renderer->CopyTexture( backbuffer, colorTarget );
	m_worldCamera->SetColorTarget( backbuffer );

	g_renderer->ReleaseRenderTarget( colorTarget );

	// Debug rendering
	if ( m_isDebugRendering )
	{
		DebugRender( gameWorld );
	}

	DebugRenderWorldToCamera( m_worldCamera );

	m_uiCamera->SetColorTarget( 0, backbuffer );
	g_renderer->BeginCamera( *m_uiCamera );

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::DebugRender( const World* gameWorld ) const
{
	RenderDebugUI();

	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 compassMatrix = Mat44::CreateTranslation3D( m_worldCamera->GetTransform().GetPosition() + .1f * m_worldCamera->GetTransform().GetForwardVector() );
	DebugAddWorldBasis( compassMatrix, .01f, 0.f, Rgba8::WHITE, Rgba8::WHITE, DEBUG_RENDER_ALWAYS );

	if ( gameWorld != nullptr )
	{
		gameWorld->DebugRender();
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::RenderDebugUI() const
{
	// Camera position and orientation
	Transform cameraTransform = m_worldCamera->GetTransform();

	std::string cameraOrientationStr = Stringf( "Yaw: %.2f, Pitch: %.2f, Roll: %.2f",
												cameraTransform.GetYawDegrees(),
												cameraTransform.GetPitchDegrees(),
												cameraTransform.GetRollDegrees() );

	std::string cameraPositionStr = Stringf( "xyz=( %.2f, %.2f, %.2f )",
											 cameraTransform.GetPosition().x,
											 cameraTransform.GetPosition().y,
											 cameraTransform.GetPosition().z );

	DebugAddScreenTextf( Vec4( 0.f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::YELLOW, 0.f,
						 "Camera - %s     %s",
						 cameraOrientationStr.c_str(),
						 cameraPositionStr.c_str() );

	// Basis text
	Mat44 cameraOrientationMatrix = cameraTransform.GetOrientationAsMatrix();
	DebugAddScreenTextf( Vec4( 0.f, .94f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::RED, 0.f,
						 "iBasis ( forward +x world east when identity  )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetIBasis3D().x,
						 cameraOrientationMatrix.GetIBasis3D().y,
						 cameraOrientationMatrix.GetIBasis3D().z );

	DebugAddScreenTextf( Vec4( 0.f, .91f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::GREEN, 0.f,
						 "jBasis ( left    +y world north when identity )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetJBasis3D().x,
						 cameraOrientationMatrix.GetJBasis3D().y,
						 cameraOrientationMatrix.GetJBasis3D().z );

	DebugAddScreenTextf( Vec4( 0.f, .88f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::BLUE, 0.f,
						 "kBasis ( up      +z world up when identity    )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetKBasis3D().x,
						 cameraOrientationMatrix.GetKBasis3D().y,
						 cameraOrientationMatrix.GetKBasis3D().z );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::RenderFPSCounter() const
{
	float fps = GetAverageFPS();

	Rgba8 fpsCountercolor = Rgba8::GREEN;

	if ( fps < 30.f )
	{
		fpsCountercolor = Rgba8::RED;
	}
	if ( fps < 55.f )
	{
		fpsCountercolor = Rgba8::YELLOW;
	}

	float frameTime = (float)m_gameClock->GetLastDeltaSeconds() * 1000.f;

	DebugAddScreenTextf( Vec4( 0.75f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, fpsCountercolor, 0.f,
						 "FPS: %.2f ( %.2f ms/frame )",
						 fps, frameTime );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::AddScreenShakeIntensity( float intensity )
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::SetCameraPositionAndYaw( const Vec2& pos, float yaw )
{
	if ( m_worldCamera == nullptr )
	{
		return;
	}

	Transform newTransform = m_worldCamera->GetTransform();
	newTransform.SetPosition( Vec3( pos, 0.5f ) );
	newTransform.SetOrientationFromPitchRollYawDegrees( 0.f, 0.f, yaw );

	m_worldCamera->SetTransform( newTransform );
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::UpdateCameraTransformToMatchPlayer( const Vec2& mouseDeltaPos )
{
	if ( m_player != nullptr )
	{
		// Rotation
		float pitchDegrees = mouseDeltaPos.y * s_mouseSensitivityMultiplier;
		pitchDegrees *= .009f;

		m_worldCamera->SetPosition( Vec3( m_player->GetPosition(), m_player->GetEyeHeight() ) );
		m_worldCamera->SetYawOrientationDegrees( m_player->GetOrientationDegrees() );
		m_worldCamera->RotateYawPitchRoll( 0.f, pitchDegrees, 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::UpdateFramesPerSecond()
{
	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT - 1; ++frameNum )
	{
		m_fpsHistory[frameNum] = m_fpsHistory[frameNum + 1];
	}

	m_fpsHistory[FRAME_HISTORY_COUNT - 1] = 1.f / (float)m_gameClock->GetLastDeltaSeconds();
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::UpdateCameras()
{

}


//-----------------------------------------------------------------------------------------------
void PlayerClient::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Vec2 forwardVec = Vec2::MakeFromPolarDegrees( m_worldCamera->GetTransform().GetYawDegrees() );
	Vec2 rightVec = forwardVec.GetRotatedMinus90Degrees();

	Vec2 translationXY( relativeTranslation.x * forwardVec
						+ relativeTranslation.y * rightVec );

	Vec3 absoluteTranslation( translationXY, relativeTranslation.z );

	m_worldCamera->Translate( absoluteTranslation );
}


//-----------------------------------------------------------------------------------------------
float PlayerClient::GetAverageFPS() const
{
	float cummulativeFPS = 0.f;
	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT; ++frameNum )
	{
		cummulativeFPS += m_fpsHistory[frameNum];
	}

	return cummulativeFPS / (float)FRAME_HISTORY_COUNT;
}


//-----------------------------------------------------------------------------------------------
EntityId PlayerClient::GetPlayerId() const
{
	return m_playerId;
}


//-----------------------------------------------------------------------------------------------
void PlayerClient::SetPlayerId( EntityId playerId )
{
	m_playerId = playerId;
	if ( m_player != nullptr )
	{
		m_player->SetId( playerId );
	}
}


//-----------------------------------------------------------------------------------------------
bool PlayerClient::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "multiplier", 1.f );

	return false;
}


//-----------------------------------------------------------------------------------------------
bool PlayerClient::SetAmbientLightColor( EventArgs* args )
{
	s_ambientLightColor = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );

	return false;
}
