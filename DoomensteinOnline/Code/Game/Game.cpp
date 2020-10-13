#include "Game/Game.hpp"
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
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputCommon.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/Entity.hpp"
#include "Game/GameJobs.hpp"
#include "Game/MapData.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
static float s_mouseSensitivityMultiplier = 1.f;
static Vec3 s_ambientLightColor = Vec3( 1.f, 1.f, 1.f );


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
	Transform::s_axisOrientation.m_axisYawPitchRollOrder = eAxisYawPitchRollOrder::ZYX;

	Transform::s_identityOrientation.PushTransform( Mat44::CreateZRotationDegrees( -90.f ) );
	Transform::s_identityOrientation.PushTransform( Mat44::CreateXRotationDegrees( 90.f ) );

	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetAmbientLightColor );
	g_eventSystem->RegisterMethodEvent( "warp", "Usage: warp <map=string> <pos=float,float> <yaw=float>", eUsageLocation::DEV_CONSOLE, this, &Game::WarpMapCommand );
			
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	m_world = new World( m_gameClock );

	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT - 1; ++frameNum )
	{
		m_fpsHistory[frameNum] = 60.f;
	}

	EnableDebugRendering();

	InitializeCameras();
	
	LoadAssets();

	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );
	BuildUIHud();

	m_curMapStr = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMapStr );
	g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMapStr.c_str() ) );
	m_world->ChangeMap( m_curMapStr );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeCameras()
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
void Game::BuildUIHud()
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
	m_worldUIPanel->AddChildPanel( gunPanelPosData,	texture, Rgba8::WHITE, uvsAtMins, uvsAtMaxs);
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	TileDefinition::s_definitions.clear();
	
	m_uiSystem->Shutdown();

	// Clean up member variables
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_gameClock );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_worldCamera );
	PTR_SAFE_DELETE( m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::Update( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos )
{
	UpdateFramesPerSecond();

	if ( !g_devConsole->IsOpen() 
		 && keyStates != nullptr ) 
	{
		UpdateFromKeyboard( keyStates, mouseDeltaPos );
	}

	m_uiSystem->Update();

	m_world->Update();

	UpdateCameraTransformToMatchPlayer( mouseDeltaPos );

	//g_jobSystem->ClaimAndDeleteAllCompletedJobs();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos )
{	
	if ( keyStates[KEY_F1].WasJustPressed() )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}

	if ( keyStates[KEY_F2].WasJustPressed() )
	{
		g_raytraceFollowCamera = !g_raytraceFollowCamera;
	}

	if ( keyStates[KEY_F3].WasJustPressed() )
	{
		if ( m_player == nullptr )
		{
			PossesNearestEntity();
		}
		else
		{
			m_player->Unpossess();
			m_player = nullptr;
		}
	}

	if ( keyStates[KEY_F4].WasJustPressed() )
	{
		g_renderer->ReloadShaders();
	}

	UpdateMovementFromKeyboard( keyStates, mouseDeltaPos );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMovementFromKeyboard( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos )
{
	Vec3 movementTranslation;
	if ( keyStates['D'].IsPressed() )
	{
		movementTranslation.y += 1.f;
	}

	if ( keyStates['A'].IsPressed() )
	{
		movementTranslation.y -= 1.f;
	}

	if ( keyStates['W'].IsPressed() )
	{
		movementTranslation.x += 1.f;
	}

	if ( keyStates['S'].IsPressed() )
	{
		movementTranslation.x -= 1.f;
	}

	if ( m_player == nullptr )
	{
		if ( keyStates['E'].IsPressed() )
		{
			movementTranslation.z += 1.f;
		}

		if ( keyStates['Q'].IsPressed() )
		{
			movementTranslation.z -= 1.f;
		}

		if ( keyStates[KEY_SHIFT].IsPressed() )
		{
			movementTranslation *= 10.f;
		}
	}

	// Rotation
	Vec2 mousePosition = mouseDeltaPos;
	float yawDegrees = -mousePosition.x * s_mouseSensitivityMultiplier;
	float pitchDegrees = mousePosition.y * s_mouseSensitivityMultiplier;
	yawDegrees *= .009f;
	pitchDegrees *= .009f;

	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	// An entity is possessed
	if ( m_player != nullptr )
	{
		// Rotation (only consider yaw so the forward vector is always in XY space)
		m_player->SetOrientationDegrees( m_player->GetOrientationDegrees() + yawDegrees );

		Vec2 forwardVec = m_player->GetForwardVector();
		Vec2 rightVec = forwardVec.GetRotatedMinus90Degrees();

		Vec2 translationXY( movementTranslation.x * forwardVec
							+ movementTranslation.y * rightVec );

		translationXY *= m_player->GetWalkSpeed();

		//m_player->AddVelocity( translationXY );
		m_player->Translate( translationXY * deltaSeconds );
	}
	// No entity possessed, move the camera directly
	else
	{
		Transform transform = m_worldCamera->GetTransform();
		m_worldCamera->RotateYawPitchRoll( yawDegrees, pitchDegrees, 0.f );

		// Translation
		TranslateCameraFPS( movementTranslation * deltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameraTransformToMatchPlayer( const Vec2& mouseDeltaPos )
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
void Game::UpdateFramesPerSecond()
{
	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT - 1; ++frameNum )
	{
		m_fpsHistory[frameNum] = m_fpsHistory[frameNum + 1];
	}

	m_fpsHistory[FRAME_HISTORY_COUNT - 1] = 1.f / (float)m_gameClock->GetLastDeltaSeconds();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{

}


//-----------------------------------------------------------------------------------------------
void Game::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Vec2 forwardVec = Vec2::MakeFromPolarDegrees( m_worldCamera->GetTransform().GetYawDegrees() );
	Vec2 rightVec = forwardVec.GetRotatedMinus90Degrees();

	Vec2 translationXY( relativeTranslation.x * forwardVec  
						+ relativeTranslation.y * rightVec );
	
	Vec3 absoluteTranslation( translationXY, relativeTranslation.z );

	m_worldCamera->Translate( absoluteTranslation );
}


//-----------------------------------------------------------------------------------------------
float Game::GetAverageFPS() const
{
	/*if constexpr ( FRAME_HISTORY_COUNT < 1 )
	{
		ERROR_AND_DIE( "FRAME_HISTORY_COUNT must be configured to be larger than 0" );
	}*/

	float cummulativeFPS = 0.f;
	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT; ++frameNum )
	{
		cummulativeFPS += m_fpsHistory[frameNum];
	}

	return cummulativeFPS / (float)FRAME_HISTORY_COUNT;
}


//-----------------------------------------------------------------------------------------------
void Game::PossesNearestEntity()
{
	Transform cameraTransform = m_worldCamera->GetTransform();

	Entity* entity = m_world->GetClosestEntityInSector( cameraTransform.GetPosition().XY(), cameraTransform.GetYawDegrees(), 90.f, 2.f );
	if ( entity == nullptr )
	{
		return;
	}

	if ( GetDistance3D( cameraTransform.GetPosition(), Vec3( entity->GetPosition(), entity->GetHeight() * .5f ) ) < 2.f )
	{
		m_player = m_world->GetClosestEntityInSector( cameraTransform.GetPosition().XY(), cameraTransform.GetYawDegrees(), 90.f, 2.f );
		m_worldCamera->SetPitchRollYawOrientationDegrees( 0.f, 0.f, m_player->GetOrientationDegrees() );

		m_player->Possess();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	RenderFPSCounter();

	Texture* backbuffer = g_renderer->GetBackBuffer();
	Texture* colorTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );

	m_worldCamera->SetColorTarget( 0, colorTarget );

	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	g_renderer->SetGamma( m_gamma );

	m_world->Render();
	
	g_renderer->EndCamera( *m_worldCamera );

	// Copy rendered data to backbuffer and set on camera
	g_renderer->CopyTexture( backbuffer, colorTarget );
	m_worldCamera->SetColorTarget( backbuffer );

	g_renderer->ReleaseRenderTarget( colorTarget );

	// Debug rendering
	if ( m_isDebugRendering )
	{
		DebugRender();
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
void Game::DebugRender() const
{
	RenderDebugUI();

	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 compassMatrix = Mat44::CreateTranslation3D( m_worldCamera->GetTransform().GetPosition() + .1f * m_worldCamera->GetTransform().GetForwardVector() );
	DebugAddWorldBasis( compassMatrix, .01f, 0.f, Rgba8::WHITE, Rgba8::WHITE, DEBUG_RENDER_ALWAYS );

	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderDebugUI() const
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
void Game::RenderFPSCounter() const
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
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	// Audio
	m_testSound = g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Teleporter.wav" );

	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Hud_Base.png" );

	SpriteSheet::CreateAndRegister( "ViewModels", *( g_renderer->CreateOrGetTextureFromFile( "Data/Images/ViewModelsSpriteSheet_8x8.png" ) ), IntVec2( 8, 8 ) );
	
	LoadXmlEntityTypes();
	LoadXmlMapMaterials();
	LoadXmlMapRegions();
	LoadXmlMaps();
	
	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlEntityTypes()
{
	g_devConsole->PrintString( "Loading Entity Types..." );

	const char* filePath = "Data/Definitions/EntityTypes.xml";

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

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		if ( !strcmp( element->Name(), "Actor" )
			 || !strcmp( element->Name(), "Entity" ) 
			 || !strcmp( element->Name(), "Projectile" ) 
			 || !strcmp( element->Name(), "Portal" ) )
		{
			EntityDefinition* entityTypeDef = new EntityDefinition( *element );
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
	
	g_devConsole->PrintString( "Entity Types Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMapMaterials()
{
	g_devConsole->PrintString( "Loading Map Materials..." );

	const char* filePath = "Data/Definitions/MapMaterialTypes.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "MapMaterialTypes" ) )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: Incorrect root node name, must be MapMaterialTypes" );
		return;
	}

	m_defaultMaterialStr = ParseXmlAttribute( *root, "default", "" );
	if ( m_defaultMaterialStr == "" )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: No default material attribute defined" );
		return;
	}

	XmlElement* materialsSheetElement = root->FirstChildElement( "MaterialsSheet" );
	if ( materialsSheetElement == nullptr )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: Must define at least one MaterialsSheet node." );
		return;
	}

	while ( materialsSheetElement )
	{
		bool materialsSheetHasError = false;
		std::string name = ParseXmlAttribute( *materialsSheetElement, "name", "" );
		if ( name == "" )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node is missing a name" ) );
			materialsSheetHasError = true;
		}
		IntVec2 layout = ParseXmlAttribute( *materialsSheetElement, "layout", IntVec2::ZERO );
		if ( layout == IntVec2::ZERO )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node '%s' is missing a layout", name.c_str() ) );
			materialsSheetHasError = true;
		}

		if ( materialsSheetHasError )
		{
			materialsSheetElement = materialsSheetElement->NextSiblingElement( "MaterialsSheet" );
			continue;
		}

		XmlElement* diffuseElement = materialsSheetElement->FirstChildElement( "Diffuse" );
		if ( diffuseElement == nullptr )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node '%s' is missing a diffuse node", name.c_str() ) );
			materialsSheetElement = materialsSheetElement->NextSiblingElement( "MaterialsSheet" );
			continue;
		}

		std::string imagePath = ParseXmlAttribute( *diffuseElement, "image", "" );
		if ( imagePath == "" )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node '%s' is missing an image", name.c_str() ) );
		}
		else
		{
			SpriteSheet::CreateAndRegister( name, *g_renderer->CreateOrGetTextureFromFile( imagePath.c_str() ), layout );
		}

		materialsSheetElement = materialsSheetElement->NextSiblingElement( "MaterialsSheet" );
	}

	if ( SpriteSheet::s_definitions.empty() )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: Must define at least one valid MaterialsSheet node." );
		return;
	}

	// Parse MaterialType nodes
	XmlElement* element = root->FirstChildElement( "MaterialType" );
	while ( element )
	{
		MapMaterialTypeDefinition* mapMaterialTypeDef = new MapMaterialTypeDefinition( *element );
		if ( mapMaterialTypeDef->IsValid() )
		{
			MapMaterialTypeDefinition::s_definitions[mapMaterialTypeDef->GetName()] = mapMaterialTypeDef;
		}

		element = element->NextSiblingElement( "MaterialType" );
	}

	if ( MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( m_defaultMaterialStr ) == nullptr )
	{
		g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: Default material '%s' was not defined as a MaterialType node.", m_defaultMaterialStr.c_str() ) );
		return;
	}

	g_devConsole->PrintString( "Map Materials Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMapRegions()
{
	g_devConsole->PrintString( "Loading Map Regions..." );
	
	const char* filePath = "Data/Definitions/MapRegionTypes.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "MapRegionTypes.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "MapRegionTypes" ) )
	{
		g_devConsole->PrintError( "MapRegionTypes.xml: Incorrect root node name, must be MapRegionTypes" );
		return;
	}

	m_defaultMapRegionStr = ParseXmlAttribute( *root, "default", "" );
	if ( m_defaultMapRegionStr == "" )
	{
		g_devConsole->PrintError( "MapRegionTypes.xml: No default region attribute defined" );
		return;
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		MapRegionTypeDefinition* mapRegionTypeDef = new MapRegionTypeDefinition( *element, m_defaultMaterialStr );
		if ( mapRegionTypeDef->IsValid() )
		{
			MapRegionTypeDefinition::s_definitions[mapRegionTypeDef->GetName()] = mapRegionTypeDef;
		}

		element = element->NextSiblingElement();
	}
	
	if ( MapRegionTypeDefinition::GetMapRegionTypeDefinition( m_defaultMapRegionStr ) == nullptr )
	{
		g_devConsole->PrintError( Stringf( "MapRegionTypes.xml: Default region '%s' was not defined as a RegionType node.", m_defaultMapRegionStr.c_str() ) );
		return;
	}

	g_devConsole->PrintString( "Map Regions Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMaps()
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

		MapData mapData( *root, GetFileNameWithoutExtension( mapName ), m_defaultMapRegionStr );

		if ( mapData.isValid )
		{
			m_world->AddNewMap( mapData );
		}
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMap( const std::string& mapName )
{
	m_world->ChangeMap( mapName );
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
void Game::SetCameraPositionAndYaw( const Vec2& pos, float yaw )
{
	Transform newTransform = m_worldCamera->GetTransform();
	newTransform.SetPosition( Vec3( pos, 0.5f ) );
	newTransform.SetOrientationFromPitchRollYawDegrees( 0.f, 0.f, yaw );

	m_worldCamera->SetTransform( newTransform );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	// No entity specified, just load the new map and set camera position and orientation
	if ( entityToWarp == nullptr )
	{
		if ( destMapName != "" )
		{
			m_world->ChangeMap( destMapName );
		}

		SetCameraPositionAndYaw( newPos, newYawDegrees );

		return;
	}

	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpMapCommand( EventArgs* args )
{
	std::string mapStr = args->GetValue( "map", "" );
	std::string posStr = args->GetValue( "pos", "" );
	std::string yawStr = args->GetValue( "yaw", "" );
	
	// Print usage and list of maps with no arguments
	if ( mapStr == ""
		 && posStr == ""
		 && yawStr == "" )
	{
		g_devConsole->PrintString( "Usage: warp <map=string> <pos=float,float> <yaw=float>" );

		std::string folderPath( "Data/Maps" );
		Strings mapFiles = GetFileNamesInFolder( folderPath, "*.xml" );

		for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
		{
			g_devConsole->PrintString( GetFileNameWithoutExtension( mapFiles[mapIdx] ) );
		}

		return;
	}

	g_devConsole->Close();

	if ( mapStr != "")
	{
		m_world->ChangeMap( mapStr );
	}

	Vec2 newPos = args->GetValue( "pos", m_worldCamera->GetTransform().GetPosition().XY() );
	float newYawDegrees = args->GetValue( "yaw", m_worldCamera->GetTransform().GetYawDegrees() );

	SetCameraPositionAndYaw( newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
bool Game::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "multiplier", 1.f );

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Game::SetAmbientLightColor( EventArgs* args )
{
	s_ambientLightColor = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );

	return false;
}
