#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexFont.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"

// For testing animations
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	PTR_SAFE_DELETE( m_spriteAtlas4x4SpriteSheet );
	PTR_SAFE_DELETE( m_spriteSheet8x2SpriteSheet );
	PTR_SAFE_DELETE( m_spriteSheet8x2AnimDefLoop );
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_worldCamera = new Camera();
	m_worldCamera->SetOutputSize( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCamera->SetPosition( Vec3( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) * .5f, 0.f ) );
	m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT );

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
	m_uiCamera->SetPosition( Vec3( WINDOW_WIDTH_PIXELS * .5f, WINDOW_HEIGHT_PIXELS * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( WINDOW_HEIGHT_PIXELS );

	m_rng = new RandomNumberGenerator();

	LoadAssets();

	m_world = new World();
	m_world->BuildNewMap( 20, 30 );

	// Create fonts
	m_fontTier3 = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/Calibri", true );
	m_fontTier4 = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/FiraCode", true );
	m_fontTier5 = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/FiraCode", true );
	m_fontTier4Material = new Material( g_renderer, "Data/Materials/FontTier4.material" );
	m_fontTier5Material = new Material( g_renderer, "Data/Materials/FontTier4.material" );
	m_fontTier5Shader = g_renderer->GetOrCreateShader( "Data/Shaders/CoolFont.shader" );
	m_fontTier5SecretShader = g_renderer->GetOrCreateShader( "Data/Shaders/CoolFont2.shader" );
	//m_fontTier3 = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/Verdana", true );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_worldCamera );
	PTR_SAFE_DELETE( m_fontTier4Material );
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
	m_worldCamera->SetOutputSize( cameraBounds.GetDimensions() );
	m_worldCamera->SetPosition( Vec3( cameraBounds.GetCenter(), 0.f ) );
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
	UpdateMousePositions();

	m_secondsSinceStart += deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera(*m_worldCamera );

	/*m_world->Render();
	if ( m_isDebugRendering )
	{
		m_world->DebugRender();
	}*/
	
	//RenderMousePointer();
	//RenderTestSpriteAnimations();

	g_renderer->EndCamera( *m_worldCamera );

	// Render UI with a new camera
	g_renderer->BeginCamera( *m_uiCamera );

	//RenderTestText();
	//RenderTestTextInBox();
		
	RenderTestTextTier3();
	RenderTestTextTier4();
	RenderTestTextTier5();

	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );
	
	g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/TestAtlas_4x4.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_SpriteSheet8x2.png" );

	m_spriteAtlas4x4Texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/TestAtlas_4x4.png" );
	m_spriteAtlas4x4SpriteSheet = new SpriteSheet( *m_spriteAtlas4x4Texture, IntVec2( 4, 4 ) );

	m_spriteSheet8x2Texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_SpriteSheet8x2.png" );
	m_spriteSheet8x2SpriteSheet = new SpriteSheet( *m_spriteSheet8x2Texture, IntVec2( 8, 2 ) );

	m_spriteSheet8x2AnimDefLoop = new SpriteAnimDefinition( *m_spriteSheet8x2SpriteSheet, 4, 9, 5.f );
	m_spriteSheet8x2AnimDefOnce = new SpriteAnimDefinition( *m_spriteSheet8x2SpriteSheet, 4, 9, 5.f, SpriteAnimPlaybackType::ONCE );
	m_spriteSheet8x2AnimDefPingPong = new SpriteAnimDefinition( *m_spriteSheet8x2SpriteSheet, 4, 9, 5.f, SpriteAnimPlaybackType::PINGPONG );


	LoadTestXml();
	LoadTestImage();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
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
void Game::RenderTestSpriteAnimations() const
{
	RenderSquareTestSprite();
	RenderNonSquareTestSprite();
	RenderNonSquareTestAnims();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderTestText() const
{
	std::vector<Vertex_PCU> textVerts;
	g_renderer->GetSystemFont()->AppendVertsForText2D( textVerts, Vec2( 200.f, 100.f ), 30.f, "Hello, world" );
	g_renderer->GetSystemFont()->AppendVertsForText2D( textVerts, Vec2( 450.f, .5f ), 50.f, "Options!", Rgba8::RED, 0.6f );
	g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
	g_renderer->DrawVertexArray( textVerts );

}


//-----------------------------------------------------------------------------------------------
void Game::RenderTestTextTier3() const
{
	std::vector<Vertex_PCU> textVerts;
	m_fontTier3->AppendVertsForText2D( textVerts, Vec2( 50.f, 1000.f ), 75.f, "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890-=_+`~!@#$%" );
	m_fontTier3->AppendVertsForText2D( textVerts, Vec2( 50.f, 900.f ), 75.f, "abcdefghijklmnopqrstuvwxyz,.';/[]}{:\"<>?\\|^&*()" );
	m_fontTier3->AppendVertsForText2D( textVerts, Vec2( 100.f, 750.f ), 150.f, "Look at my cool tier 3 font!" );
	g_renderer->BindTexture( 0, m_fontTier3->GetTexture() );
	g_renderer->DrawVertexArray( textVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderTestTextTier4() const
{
	std::vector<Vertex_PCU> textVerts;
	m_fontTier4->AppendVertsForText2D( textVerts, Vec2( 50.f, 300.f ), 50.f, "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890-=_+`~!@#$%", Rgba8::GREEN );
	m_fontTier4->AppendVertsForText2D( textVerts, Vec2( 50.f, 200.f ), 50.f, "abcdefghijklmnopqrstuvwxyz,.';/[]}{:\"<>?\\|^&*()", Rgba8::GREEN );
	m_fontTier4->AppendVertsForText2D( textVerts, Vec2( 100.f, 50.f ), 100.f, "Look at my even cooler tier 4 font!", Rgba8::GREEN );
	g_renderer->BindMaterial( m_fontTier4Material );
	g_renderer->BindDiffuseTexture( m_fontTier4->GetTexture() );
	g_renderer->DrawVertexArray( textVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderTestTextTier5() const
{
	std::vector<VertexFont> textVerts;

	std::string text( "Behold the fabled tier 5 font..." );
	Vec4 specialEffects( (float)text.length(), 0.f, 0.f, 0.f );
	m_fontTier5->AppendVertsForText2D( textVerts, Vec2( 100.f, 550.f ), 110.f, text, Rgba8::WHITE, specialEffects );
	g_renderer->BindMaterial( m_fontTier5Material );
	g_renderer->BindDiffuseTexture( m_fontTier5->GetTexture() );
	
	g_renderer->BindShader( m_fontTier5Shader );
	g_renderer->DrawVertexArray( textVerts );

	
	textVerts.clear();
	specialEffects = Vec4( m_mouseUIPosition.x, WINDOW_HEIGHT_PIXELS - m_mouseUIPosition.y, 0.f, 0.f );
	m_fontTier5->AppendVertsForText2D( textVerts, Vec2( 20.f, 500.f ), 75.f, "You found me!", Rgba8::WHITE, specialEffects );
	g_renderer->BindMaterial( m_fontTier5Material );
	g_renderer->BindDiffuseTexture( m_fontTier5->GetTexture() );

	g_renderer->BindShader( m_fontTier5SecretShader );
	g_renderer->DrawVertexArray( textVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderTestTextInBox() const
{

	AABB2 box( Vec2( 1000.f, 40.f ), Vec2( 1800.f, 750.f ) );

	g_renderer->BindTexture( 0, nullptr );
	DrawAABB2( g_renderer, box, Rgba8(127, 0, 255, 100 ));

	std::vector<Vertex_PCU> textVerts;
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Center", Rgba8::GREEN, 1.f, ALIGN_CENTERED );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Bottom Left", Rgba8::GREEN, 1.f, ALIGN_BOTTOM_LEFT );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Bottom Center", Rgba8::GREEN, 1.f, ALIGN_BOTTOM_CENTER );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Bottom Right", Rgba8::GREEN, 1.f, ALIGN_BOTTOM_RIGHT );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Center Left", Rgba8::GREEN, 1.f, ALIGN_CENTERED_LEFT );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Center Right", Rgba8::GREEN, 1.f, ALIGN_CENTERED_RIGHT );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Top Left", Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Top Center", Rgba8::GREEN, 1.f, ALIGN_TOP_CENTER );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Top Right", Rgba8::GREEN, 1.f, ALIGN_TOP_RIGHT );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 32.f, "Somewhere in the Middle", Rgba8::RED, 1.f, Vec2( .3f, .7f ) );
	g_renderer->GetSystemFont()->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Hi", Rgba8::RED, 1.f, Vec2( .15f, .15f ) );
	g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
	g_renderer->DrawVertexArray( textVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderSquareTestSprite() const
{
	const SpriteDefinition& testSpriteDef = m_spriteAtlas4x4SpriteSheet->GetSpriteDefinition( 2 );

	Vec2 uvAtMins, uvAtMaxs;
	testSpriteDef.GetUVs( uvAtMins, uvAtMaxs );
	std::vector<Vertex_PCU> testVerts;

	float inverseAspect = 1.f / testSpriteDef.GetAspect();
	AABB2 spriteBounds( AABB2::ONE_BY_ONE );
	spriteBounds.maxs.x *= testSpriteDef.GetAspect();
	spriteBounds.maxs.y *= inverseAspect;

	AppendVertsForAABB2D( testVerts, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );

	Vertex_PCU::TransformVertexArray( testVerts, 1.f, 0.f, Vec2( 1.f, 5.f ) );

	g_renderer->BindTexture( 0, m_spriteAtlas4x4Texture );
	g_renderer->DrawVertexArray( testVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderNonSquareTestSprite() const
{
	const SpriteDefinition& testSpriteDef = m_spriteSheet8x2SpriteSheet->GetSpriteDefinition( 2 );

	Vec2 uvAtMins, uvAtMaxs;
	testSpriteDef.GetUVs( uvAtMins, uvAtMaxs );
	std::vector<Vertex_PCU> testVerts;

	float inverseAspect = 1.f / testSpriteDef.GetAspect();
	AABB2 spriteBounds( AABB2::ONE_BY_ONE );
	spriteBounds.maxs.x *= testSpriteDef.GetAspect();
	spriteBounds.maxs.y *= inverseAspect;

	AppendVertsForAABB2D( testVerts, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );

	Vertex_PCU::TransformVertexArray( testVerts, 1.f, 0.f, Vec2( 2.5f, 5.f ) );

	g_renderer->BindTexture( 0, m_spriteSheet8x2Texture );
	g_renderer->DrawVertexArray( testVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderMousePointer() const
{
	Vec2 mouseWorldPosition( g_inputSystem->GetNormalizedMouseClientPos() );
	mouseWorldPosition.x *= WINDOW_WIDTH;
	mouseWorldPosition.y *= WINDOW_HEIGHT;

	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, mouseWorldPosition, .05f, Rgba8::CYAN, .05f );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderSpriteAnimation( const SpriteDefinition& spriteDef, const Vec2& position ) const
{
	Vec2 uvAtMins, uvAtMaxs;
	spriteDef.GetUVs( uvAtMins, uvAtMaxs );
	std::vector<Vertex_PCU> testVerts;

	float inverseAspect = 1.f / spriteDef.GetAspect();
	AABB2 spriteBounds( AABB2::ONE_BY_ONE );
	spriteBounds.maxs.x *= spriteDef.GetAspect();
	spriteBounds.maxs.y *= inverseAspect;

	AppendVertsForAABB2D( testVerts, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );

	Vertex_PCU::TransformVertexArray( testVerts, 1.f, 0.f, position );

	g_renderer->BindTexture( 0, m_spriteSheet8x2Texture );
	g_renderer->DrawVertexArray( testVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderNonSquareTestAnims() const
{
	RenderSpriteAnimation( m_spriteSheet8x2AnimDefLoop->GetSpriteDefAtTime( m_secondsSinceStart ), Vec2( 4.f, 5.f ) );
	RenderSpriteAnimation(m_spriteSheet8x2AnimDefOnce->GetSpriteDefAtTime( m_secondsSinceStart ), Vec2( 6.f, 5.f ) );
	RenderSpriteAnimation(m_spriteSheet8x2AnimDefPingPong->GetSpriteDefAtTime( m_secondsSinceStart ), Vec2( 7.f, 5.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTestXml()
{
	const char* filePath = "Data/Gameplay/TileDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintString( "XML file couldn't be opened!", Rgba8::RED );
		return;
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		g_devConsole->PrintString( ParseXmlAttribute( *element, "name", "" ) );
		g_devConsole->PrintString( ParseXmlAttribute( *element, "spriteCoords", "" ) );
		g_devConsole->PrintString( ParseXmlAttribute( *element, "spriteTint", "" ) );
		g_devConsole->PrintString( ParseXmlAttribute( *element, "allowsSight", "" ) );
		g_devConsole->PrintString( ParseXmlAttribute( *element, "allowsWalking", "" ) );
		g_devConsole->PrintString( ParseXmlAttribute( *element, "allowsFlying", "" ) );
		g_devConsole->PrintString( ParseXmlAttribute( *element, "allowsSwimming", "" ) );

		element = element->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTestImage()
{
	Image testImage( "Data/Images/Test_StbiFlippedAndOpenGL.png" );

	GUARANTEE_OR_DIE( testImage.GetTexelColor( 0, 0 ) == Rgba8::RED, " Lower left texel isn't red " );
	GUARANTEE_OR_DIE( testImage.GetTexelColor( 1, 0 ) == Rgba8::GREEN, " Lower left texel isn't green " );
	GUARANTEE_OR_DIE( testImage.GetTexelColor( 2, 0 ) == Rgba8::BLUE, " Lower left texel isn't blue " );

	GUARANTEE_OR_DIE( testImage.GetTexelColor( 0, testImage.GetDimensions().y - 1 ) == Rgba8::CYAN, " Top left texel isn't cyan " );
	GUARANTEE_OR_DIE( testImage.GetTexelColor( 1, testImage.GetDimensions().y - 1 ) == Rgba8::MAGENTA, " Top left texel isn't magenta " );
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
		SoundID testSound = g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_audioSystem->PlaySound( testSound );
	}

	if ( g_inputSystem->WasKeyJustPressed( 'S' ) )
	{
		g_eventSystem->RegisterEvent( "LogToDevConsole", "", eUsageLocation::EVERYWHERE, LogToDevConsole );
	}

	if ( g_inputSystem->WasKeyJustPressed( 'D' ) )
	{
		g_eventSystem->DeRegisterEvent( "LogToDevConsole", LogToDevConsole );
	}

	if ( g_inputSystem->WasKeyJustPressed( '0' ) )
	{
		EventArgs args;
		args.SetValue("Color", "0,255,0");
		args.SetValue("Message", "Logged by event");

		g_eventSystem->FireEvent( "LogToDevConsole", &args );
	}

	if ( g_inputSystem->WasKeyJustPressed( '9' ) )
	{
		g_devConsole->PrintString( "Hey, Red", Rgba8::RED );
	}

	if ( g_inputSystem->WasKeyJustPressed( '8' ) )
	{
		g_devConsole->PrintString( "Yo, Purple", Rgba8::PURPLE );
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_TILDE ) )
	{
		g_devConsole->ToggleOpenFull();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * deltaSeconds;
	m_screenShakeIntensity = ClampMinMax(m_screenShakeIntensity, 0.f, 1.f);

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_rng->RollRandomFloatInRange(-maxScreenShake, maxScreenShake);
	float cameraShakeY = m_rng->RollRandomFloatInRange(-maxScreenShake, maxScreenShake);
	Vec2 cameraShakeOffset = Vec2(cameraShakeX, cameraShakeY);

	m_worldCamera->Translate2D(cameraShakeOffset);
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
bool Game::LogToDevConsole( EventArgs* args )
{
	Rgba8 color( Rgba8::WHITE );
	color = args->GetValue( "Color", color );

	std::string message;
	message = args->GetValue( "Message", message );

	g_devConsole->PrintString( message, color );
	return false;
}
