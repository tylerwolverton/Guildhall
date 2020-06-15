#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Item.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ItemDefinition.hpp"
#include "Game/UIButton.hpp"
#include "Game/UIPanel.hpp"


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
	m_worldCamera->SetOutputSize( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCamera->SetPosition( m_focalPoint );

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
	m_uiCamera->SetPosition( Vec3( WINDOW_WIDTH_PIXELS * .5f, WINDOW_HEIGHT_PIXELS * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( WINDOW_HEIGHT_PIXELS );

	EnableDebugRendering();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, true );

	BuildHUD();
	UpdateInventoryButtonImages();

	m_world = new World( m_gameClock );
	  
	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::BeginFrame()
{
	
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	TileDefinition::s_definitions.clear();

	CleanupHUD();

	// Clean up global sprite sheets
	PTR_SAFE_DELETE( g_tileSpriteSheet );
	PTR_SAFE_DELETE( g_characterSpriteSheet );
	PTR_SAFE_DELETE( g_portraitSpriteSheet );
	
	//Clean up spritesheets
	SpriteSheet::DeleteSpriteSheets();


	// Clean up member variables
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_worldCamera );
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
	g_devConsole->PrintString( "Map Generation Debug Commands" );
	g_devConsole->PrintString( "F4 - View entire map" );
	g_devConsole->PrintString( "F5 - Reload current map" );
}


////-----------------------------------------------------------------------------------------------
//void Game::SetWorldCameraOrthographicView( const AABB2& cameraBounds )
//{
//	m_worldCamera->SetOrthoView( cameraBounds.mins, cameraBounds.maxs );
//}
//
//
////-----------------------------------------------------------------------------------------------
//void Game::SetWorldCameraOrthographicView( const Vec2& bottomLeft, const Vec2& topRight )
//{
//	SetWorldCameraOrthographicView( AABB2( bottomLeft, topRight ) );
//}


//-----------------------------------------------------------------------------------------------
void Game::Update()
{
	switch ( m_gameState )
	{
		case eGameState::LOADING:
		{
			switch ( m_loadingFrameNum )
			{
				case 0:
				{
					++m_loadingFrameNum;

					SoundID anticipation = g_audioSystem->CreateOrGetSound( "Data/Audio/Anticipation.mp3" );
					g_audioSystem->PlaySound( anticipation, false, .25f );
				}
				break;

				case 1:
				{
					LoadAssets();
					ChangeGameState( eGameState::ATTRACT );
					Update();
				}
				break;
			}
		}
		break;

		case eGameState::ATTRACT:
		{
			UpdateFromKeyboard();
		}
		break;

		case eGameState::PLAYING:
		{
			UpdateFromKeyboard();

			m_world->Update();
		}
		break;
	}

	UpdateCameras();
	UpdateMousePositions();

	m_rootPanel->Update();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_gameState )
	{
		case eGameState::PLAYING:
		case eGameState::PAUSED:
		{
			m_world->Render();
			if ( m_isDebugRendering )
			{
				m_world->DebugRender();
			}
		}
		break;
	}

	g_renderer->EndCamera( *m_worldCamera );

	g_renderer->BeginCamera( *m_uiCamera );

	switch ( m_gameState )
	{
		case eGameState::LOADING:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Loading..." );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;

		case eGameState::ATTRACT:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 100.f, 500.f ), 60.f, "The Tentacle of Monkey Island" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 400.f ), 30.f, "Press Any Key to Start" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;

		case eGameState::PLAYING:
		{
			//m_world->RenderHUD();
		}
		break;
	}

	m_rootPanel->Render( g_renderer );

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Game::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );
	g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );

	// Music
	g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/GameOver.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheScummBar.mp3" );

	// TODO: Check for nullptrs when loading textures
	//g_tileSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_32x32.png" )), IntVec2( 32, 32 ) );
	//g_characterSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaCharacters_12x53.png" )), IntVec2( 12, 53 ) );
	//g_portraitSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaPortraits_8x8.png" )), IntVec2( 8, 8 ) );

	g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" );
	g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/DebugRender.hlsl" );

	LoadActorsFromXml();
	LoadItemsFromXml();
	LoadMapsFromXml();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadMapsFromXml()
{
	g_devConsole->PrintString( "Loading Maps..." );

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

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadActorsFromXml()
{
	g_devConsole->PrintString( "Loading Actors..." );

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

	g_devConsole->PrintString( "Actors Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadItemsFromXml()
{
	g_devConsole->PrintString( "Loading Items..." );

	const char* filePath = "Data/Gameplay/ItemDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The items xml file '%s' could not be opened.", filePath ) );
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		ItemDefinition* itemDef = new ItemDefinition( *element );
		ItemDefinition::s_definitions[itemDef->GetName()] = itemDef;

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Items Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			if ( g_inputSystem->WasAnyKeyJustPressed() )
			{
				ChangeGameState( eGameState::PLAYING );
			}
		}
		break;

		case eGameState::PLAYING:
		{
			m_isSlowMo = g_inputSystem->IsKeyPressed( 'T' );
			m_isFastMo = g_inputSystem->IsKeyPressed( 'Y' );

			if ( g_inputSystem->WasKeyJustPressed( 'P' ) )
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
		}
		break;

	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadNewMap( const std::string& mapName )
{
	delete m_world;
	m_world = nullptr;
	m_world = new World( m_gameClock );
	m_world->BuildNewMap( mapName );
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
void Game::UpdateCameras()
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * (float)m_gameClock->GetLastDeltaSeconds();
	m_screenShakeIntensity = ClampZeroToOne( m_screenShakeIntensity );

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	float cameraShakeY = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	Vec2 cameraShakeOffset = Vec2( cameraShakeX, cameraShakeY );

	//m_worldCamera->Translate2D( cameraShakeOffset );
	m_worldCamera->SetPosition( m_focalPoint + Vec3( cameraShakeOffset, 0.f ) );
	m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::SetWorldCameraPosition( const Vec3& position )
{
	m_focalPoint = position;
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
void Game::ChangeGameState( const eGameState& newGameState )
{
	switch ( newGameState )
	{
		case eGameState::LOADING:
		{
			ERROR_AND_DIE( "Tried to go back to the loading state during the game. Don't do that." );
		}
		break;

		case eGameState::ATTRACT:
		{
			m_hudPanel->Deactivate();
			m_hudPanel->Hide();

			// Check which state we are changing from
			switch ( m_gameState )
			{
				case eGameState::PAUSED:
				case eGameState::PLAYING:
				{
					g_audioSystem->StopSound( m_gameplayMusicID );
				}
				break;

				case eGameState::VICTORY:
				{
					g_audioSystem->StopSound( m_victoryMusicID );
				}
				break;
			}

			SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
			m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true, .25f );
		}
		break;

		case eGameState::PLAYING:
		{
			m_hudPanel->Activate();
			m_hudPanel->Show();

			// Check which state we are changing from
			switch ( m_gameState )
			{
				case eGameState::PAUSED:
				{
					SoundID unpause = g_audioSystem->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
					g_audioSystem->PlaySound( unpause );

					g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, 1.f );
				}
				break;

				case eGameState::ATTRACT:
				{
					g_audioSystem->StopSound( m_attractMusicID );

					SoundID gameplayMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheScummBar.mp3" );
					m_gameplayMusicID = g_audioSystem->PlaySound( gameplayMusic, true, .25f );
					
					m_curMap = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMap );
					g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMap.c_str() ) );
					ChangeMap( m_curMap );
					//m_world->BuildNewMap( m_curMap );
				}
				break;
			}
		}
		break;

		case eGameState::PAUSED:
		{
			g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, .5f );

			SoundID pause = g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
			g_audioSystem->PlaySound( pause );

		}
		break;

		case eGameState::VICTORY:
		{
			//m_curVictoryScreenSeconds = 0.f;
			m_hudPanel->Deactivate();
			m_hudPanel->Hide();

			g_audioSystem->StopSound( m_gameplayMusicID );

			SoundID victoryMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
			m_victoryMusicID = g_audioSystem->PlaySound( victoryMusic );
		}
		break;
	}

	m_gameState = newGameState;
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMap( const std::string& mapName )
{
	m_world->BuildNewMap( mapName );

	if ( mapName == "Victory" )
	{
		m_hudPanel->Deactivate();
		m_hudPanel->Hide();

		m_rootPanel->SetBackgroundTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/TheTentacleOfMonkeyIsland-logo.png" ) );

		m_focalPoint = Vec3( WINDOW_WIDTH * .5f, WINDOW_HEIGHT * .5f, 0.f);
	}
}


//-----------------------------------------------------------------------------------------------
void Game::BuildHUD()
{
	//Texture* rootBackground = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	Texture* childBackground = g_renderer->GetDefaultWhiteTexture();

	m_rootPanel = new UIPanel( AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ) );

	m_hudPanel = m_rootPanel->AddChildPanel( Vec2( 0.f, 1.f ), Vec2( 0.f, .25f ), childBackground, Rgba8::RED );

	BuildVerbPanel();
	BuildInventoryPanel();

	m_hudPanel->Deactivate();
	m_hudPanel->Hide();

}


//-----------------------------------------------------------------------------------------------
void Game::CleanupHUD()
{
	PTR_SAFE_DELETE( m_rootPanel );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildVerbPanel()
{
	Texture* background = g_renderer->GetDefaultWhiteTexture();

	m_verbPanel = m_hudPanel->AddChildPanel( Vec2( 0.f, .5f ), Vec2( 0.f, 1.f ), background, Rgba8::BLACK );

	m_giveVerbButton = m_verbPanel->AddButton( Vec2( 0.01f, 0.52f ), Vec2( 0.32f, .48f ), background, Rgba8::DARK_BLUE );
	m_giveVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_giveVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_giveVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_giveVerbButton->AddText( Vec2( .1f, .1f ), Vec2( .8f, .8f ), "Give" );

	m_openVerbButton = m_verbPanel->AddButton( Vec2( .34f, 0.52f ), Vec2( 0.32f, .48f ), background, Rgba8::DARK_BLUE );
	m_openVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_openVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_openVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_openVerbButton->AddText( Vec2( .1f, .1f ), Vec2( .8f, .8f ), "Open" );

	m_closeVerbButton = m_verbPanel->AddButton( Vec2( .67f, 0.52f ), Vec2( 0.32f, .48f ), background, Rgba8::DARK_BLUE );
	m_closeVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_closeVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_closeVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_closeVerbButton->AddText( Vec2( .1f, .1f ), Vec2( .8f, .8f ), "Close" );

	m_pickUpVerbButton = m_verbPanel->AddButton( Vec2( .01f, .02f ), Vec2( 0.49f, .48f ), background, Rgba8::DARK_BLUE );
	m_pickUpVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_pickUpVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_pickUpVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_pickUpVerbButton->AddText( Vec2( .1f, .1f ), Vec2( .8f, .8f ), "Pick up" );

	m_talkToVerbButton = m_verbPanel->AddButton( Vec2( .51f, .02f ), Vec2( 0.48f, .48f ), background, Rgba8::DARK_BLUE );
	m_talkToVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
	m_talkToVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_talkToVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_talkToVerbButton->AddText( Vec2( .1f, .1f ), Vec2( .8f, .8f ), "Talk to" );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildInventoryPanel()
{
	Texture* background = g_renderer->GetDefaultWhiteTexture();

	m_inventoryPanel = m_hudPanel->AddChildPanel( Vec2( .5f, 1.f ), Vec2( 0.f, 1.f ), background, Rgba8::BLACK );

	constexpr int NUM_IN_ROW = 4;
	constexpr int NUM_ROWS = 2;
	constexpr float rowHeightFraction = 1.f / (float)NUM_ROWS;
	constexpr float rowWidthFraction = 1.f / (float)NUM_IN_ROW;
	for ( int buttonRowNum = 0; buttonRowNum < NUM_ROWS; ++buttonRowNum )
	{
		for ( int buttonColumnNum = 0; buttonColumnNum < NUM_IN_ROW; ++buttonColumnNum )
		{
			float widthStart = (float)buttonColumnNum * rowWidthFraction;
			float heightStart = 1.f - ( (float)buttonRowNum + 1.f ) * rowHeightFraction;

			widthStart = ClampZeroToOne( widthStart );
			heightStart = ClampZeroToOne( heightStart ) + .02f;

			Vec2 relativeMinPosition( widthStart, heightStart );
			Vec2 relativeFractionOfDimensions( rowWidthFraction - .01f, rowHeightFraction - .02f );

			UIButton* inventoryButton = m_inventoryPanel->AddButton( relativeMinPosition, relativeFractionOfDimensions, background, Rgba8::DARK_BLUE );
			inventoryButton->m_onClickEvent.SubscribeMethod( this, &Game::OnTestButtonClicked );
			inventoryButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
			inventoryButton->m_onHoverStayEvent.SubscribeMethod( this, &Game::OnInventoryItemHoverStay );
			inventoryButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );

			/*std::string str = "Button";
			str.append( ToString(i++) );
			inventoryButton->AddText( Vec2( .1f, .1f ), Vec2( .8f, .8f ), str );*/

			m_inventoryButtons.push_back( inventoryButton );
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Button events
//-----------------------------------------------------------------------------------------------
void Game::OnTestButtonClicked( EventArgs* args )
{
	UNUSED( args );

	/*SoundID anticipation = g_audioSystem->CreateOrGetSound( "Data/Audio/Anticipation.mp3" );
	g_audioSystem->PlaySound( anticipation, false, .25f );*/

	//Mat44::CreateTranslation2D( m_testButton->GetPosition() + Vec2( 0.f, 1.f );
	DebugAddWorldTextf( Mat44::CreateTranslation2D( Vec2( 5.f, 5.f ) ),
						Vec2( .5f, .5f ),
						Rgba8::GREEN,
						1.f,
						.1f,
						DEBUG_RENDER_ALWAYS,
						"Button clicked!");
}


//-----------------------------------------------------------------------------------------------
void Game::OnInventoryItemHoverStay( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			std::string itemName = m_inventory[inventoryButtonIdx]->GetName();

			DebugAddScreenText( Vec4( g_inputSystem->GetNormalizedMouseClientPos(), Vec2::ZERO ),
								Vec2( .5f, .5f ),
								20.f,
								Rgba8::WHITE,
								Rgba8::WHITE,
								0.f,
								itemName.c_str() );
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnTestButtonHoverBegin( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );
	
	Rgba8 tint = Rgba8::DARK_BLUE;
	tint.r += 10;
	tint.g += 10;
	tint.b += 10;

	if ( id == m_giveVerbButton->GetId() )	 { m_giveVerbButton->SetTint( tint );	return; }
	if ( id == m_openVerbButton->GetId() )	 { m_openVerbButton->SetTint( tint );	return; }
	if ( id == m_closeVerbButton->GetId() )  { m_closeVerbButton->SetTint( tint );	return; }
	if ( id == m_pickUpVerbButton->GetId() ) { m_pickUpVerbButton->SetTint( tint ); return; }
	if ( id == m_talkToVerbButton->GetId() ) { m_talkToVerbButton->SetTint( tint ); return; }

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			itemButton->SetTint( tint );
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnTestButtonHoverEnd( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );

	Rgba8 tint = Rgba8::DARK_BLUE;

	if ( id == m_giveVerbButton->GetId() ) { m_giveVerbButton->SetTint( tint );	return; }
	if ( id == m_openVerbButton->GetId() ) { m_openVerbButton->SetTint( tint );	return; }
	if ( id == m_closeVerbButton->GetId() ) { m_closeVerbButton->SetTint( tint );	return; }
	if ( id == m_pickUpVerbButton->GetId() ) { m_pickUpVerbButton->SetTint( tint ); return; }
	if ( id == m_talkToVerbButton->GetId() ) { m_talkToVerbButton->SetTint( tint ); return; }

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			itemButton->SetTint( tint );
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnGiveButtonClicked( EventArgs* args )
{
	UNUSED( args );
}


//-----------------------------------------------------------------------------------------------
void Game::OnOpenButtonClicked( EventArgs* args )
{
	UNUSED( args );
}


//-----------------------------------------------------------------------------------------------
void Game::OnCloseButtonClicked( EventArgs* args )
{
	UNUSED( args );
}


//-----------------------------------------------------------------------------------------------
void Game::OnPickUpButtonClicked( EventArgs* args )
{
	UNUSED( args );
}


//-----------------------------------------------------------------------------------------------
void Game::OnTalkToButtonClicked( EventArgs* args )
{
	UNUSED( args );
}


//-----------------------------------------------------------------------------------------------
void Game::AddItemToInventory( Item* newItem )
{
	m_inventory.push_back( newItem );
	newItem->SetIsInPlayerInventory( true );

	UpdateInventoryButtonImages();
}


//-----------------------------------------------------------------------------------------------
void Game::RemoveItemFromInventory( Item* itemToRemove )
{
	int itemIdx = 0;
	for ( ; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] == itemToRemove )
		{
			break;
		}
	}

	if ( itemIdx < (int)m_inventory.size() )
	{
		m_inventory.erase( m_inventory.begin() + itemIdx );
	
		UpdateInventoryButtonImages();

		itemToRemove->SetIsInPlayerInventory( false );
	}
}


//-----------------------------------------------------------------------------------------------
bool Game::IsItemInInventory( Item* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] == item )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Game::PickupAtMousePosition()
{
	Vec2 clickPosition = g_game->GetMouseWorldPosition();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateInventoryButtonImages()
{
	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		if ( inventoryButtonIdx < (int)m_inventory.size() )
		{
			SpriteDefinition* spriteDef = m_inventory[inventoryButtonIdx]->GetSpriteDef();
			m_inventoryButtons[inventoryButtonIdx]->AddImage( Vec2( .1f, .1f ), Vec2( .8f, .8f ), spriteDef );
			m_inventoryButtons[inventoryButtonIdx]->Activate();
		}
		// Clear Image
		else
		{
			m_inventoryButtons[inventoryButtonIdx]->ClearLabels();
			m_inventoryButtons[inventoryButtonIdx]->Deactivate();
		}
	}

}
