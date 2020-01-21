#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
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
	delete m_spriteAtlas4x4SpriteSheet;
	m_spriteAtlas4x4SpriteSheet = nullptr;

	delete m_spriteSheet8x2SpriteSheet;
	m_spriteSheet8x2SpriteSheet = nullptr;

	delete m_spriteSheet8x2AnimDefLoop;
	m_spriteSheet8x2AnimDefLoop = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_worldCamera = new Camera();
	m_uiCamera = new Camera();

	m_rng = new RandomNumberGenerator();

	LoadAssets();

	m_world = new World();
	m_world->BuildNewMap( 20, 30 );

	g_devConsole->PrintString( Rgba8::GREEN, "Game Started" );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_world;
	m_world = nullptr;

	delete m_rng;
	m_rng = nullptr;
	
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

	m_secondsSinceStart += deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_renderer->ClearScreen(Rgba8(0, 0, 0));

	g_renderer->BeginCamera(*m_worldCamera );

	/*m_world->Render();
	if ( m_isDebugRendering )
	{
		m_world->DebugRender();
	}*/
	
	RenderMousePointer();
	RenderTestSpriteAnimations();

	g_renderer->EndCamera( *m_worldCamera );

	// Render UI with a new camera
	g_renderer->BeginCamera( *m_uiCamera );

	RenderTestText();
	RenderTestTextInBox();

	g_devConsole->Render( *g_renderer, *m_uiCamera, 20 );
	
	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( Rgba8::WHITE, "Loading Assets..." );

	g_testFont = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

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

	g_devConsole->PrintString( Rgba8::GREEN, "Assets Loaded" );
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
	g_testFont->AppendVertsForText2D( textVerts, Vec2( 200.f, 100.f ), 30.f, "Hello, world" );
	g_testFont->AppendVertsForText2D( textVerts, Vec2( 450.f, .5f ), 50.f, "Options!", Rgba8::RED, 0.6f );
	g_renderer->BindTexture( g_testFont->GetTexture() );
	g_renderer->DrawVertexArray( textVerts );

}


//-----------------------------------------------------------------------------------------------
void Game::RenderTestTextInBox() const
{

	AABB2 box( Vec2( 1000.f, 40.f ), Vec2( 1800.f, 750.f ) );

	g_renderer->BindTexture( nullptr );
	g_renderer->DrawAABB2( box, Rgba8(127, 0, 255, 100 ));

	std::vector<Vertex_PCU> textVerts;
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Center", Rgba8::GREEN, 1.f, ALIGN_CENTERED );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Bottom Left", Rgba8::GREEN, 1.f, ALIGN_BOTTOM_LEFT );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Bottom Center", Rgba8::GREEN, 1.f, ALIGN_BOTTOM_CENTER );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Bottom Right", Rgba8::GREEN, 1.f, ALIGN_BOTTOM_RIGHT );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Center Left", Rgba8::GREEN, 1.f, ALIGN_CENTERED_LEFT );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Center Right", Rgba8::GREEN, 1.f, ALIGN_CENTERED_RIGHT );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Top Left", Rgba8::GREEN, 1.f, ALIGN_TOP_LEFT );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Top Center", Rgba8::GREEN, 1.f, ALIGN_TOP_CENTER );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Top Right", Rgba8::GREEN, 1.f, ALIGN_TOP_RIGHT );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 32.f, "Somewhere in the Middle", Rgba8::RED, 1.f, Vec2( .3f, .7f ) );
	g_testFont->AppendVertsForTextInBox2D( textVerts, box, 16.f, "Hi", Rgba8::RED, 1.f, Vec2( .15f, .15f ) );
	g_renderer->BindTexture( g_testFont->GetTexture() );
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

	g_renderer->AppendVertsForAABB2D( testVerts, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );

	Vertex_PCU::TransformVertexArray( testVerts, 1.f, 0.f, Vec2( 1.f, 5.f ) );

	g_renderer->BindTexture( m_spriteAtlas4x4Texture );
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

	g_renderer->AppendVertsForAABB2D( testVerts, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );

	Vertex_PCU::TransformVertexArray( testVerts, 1.f, 0.f, Vec2( 2.5f, 5.f ) );

	g_renderer->BindTexture( m_spriteSheet8x2Texture );
	g_renderer->DrawVertexArray( testVerts );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderMousePointer() const
{
	Vec2 mouseWorldPosition( g_inputSystem->GetNormalizedMouseClientPos() );
	mouseWorldPosition.x *= WINDOW_WIDTH;
	mouseWorldPosition.y *= WINDOW_HEIGHT;

	g_renderer->BindTexture( nullptr );
	g_renderer->DrawRing2D( mouseWorldPosition, .05f, Rgba8::CYAN, .05f );
}


void Game::RenderSpriteAnimation( const SpriteDefinition& spriteDef, const Vec2& position ) const
{
	Vec2 uvAtMins, uvAtMaxs;
	spriteDef.GetUVs( uvAtMins, uvAtMaxs );
	std::vector<Vertex_PCU> testVerts;

	float inverseAspect = 1.f / spriteDef.GetAspect();
	AABB2 spriteBounds( AABB2::ONE_BY_ONE );
	spriteBounds.maxs.x *= spriteDef.GetAspect();
	spriteBounds.maxs.y *= inverseAspect;

	g_renderer->AppendVertsForAABB2D( testVerts, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );

	Vertex_PCU::TransformVertexArray( testVerts, 1.f, 0.f, position );

	g_renderer->BindTexture( m_spriteSheet8x2Texture );
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
		g_devConsole->PrintString( Rgba8::RED, "XML file couldn't be opened!" );
		return;
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "name", "" ) );
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "spriteCoords", "" ) );
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "spriteTint", "" ) );
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "allowsSight", "" ) );
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "allowsWalking", "" ) );
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "allowsFlying", "" ) );
		g_devConsole->PrintString( Rgba8::WHITE, ParseXmlAttribute( *element, "allowsSwimming", "" ) );

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
		g_eventSystem->RegisterEvent( "LogToDevConsole", LogToDevConsole );
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
		g_devConsole->PrintString( Rgba8::RED, "Hey, Red" );
	}

	if ( g_inputSystem->WasKeyJustPressed( '8' ) )
	{
		g_devConsole->PrintString( Rgba8::PURPLE, "Yo, Purple" );
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
bool Game::LogToDevConsole( EventArgs args )
{
	Rgba8 color( Rgba8::WHITE );
	color = args.GetValue( "Color", color );

	std::string message;
	message = args.GetValue( "Message", message );

	g_devConsole->PrintString( color, message );
	return false;
}
