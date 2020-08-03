#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/Entity.hpp"
#include "Game/Actor.hpp"
#include "Game/Cursor.hpp"
#include "Game/Portal.hpp"
#include "Game/World.hpp"
#include "Game/DialogueState.hpp"
#include "Game/MapDefinition.hpp"


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

	Vec2 windowDimensions = g_window->GetDimensions();
	
	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );

	EnableDebugRendering();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );
	
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, false, true );

	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );
	BuildHUD();
	BuildMenus();
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
	StopAllSounds();
	
	g_audioSystem->Shutdown();

	m_uiSystem->Shutdown();

	// Cleanup player and cursor
	PTR_SAFE_DELETE( m_player );
	PTR_SAFE_DELETE( m_cursor );

	// Clean up global sprite sheets
	PTR_SAFE_DELETE( g_tileSpriteSheet );
	PTR_SAFE_DELETE( g_characterSpriteSheet );
	PTR_SAFE_DELETE( g_portraitSpriteSheet );
	
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

					SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheLookout.mp3" );
					m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true, .25f );
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
		case eGameState::PAUSED:
		case eGameState::VICTORY:
		{
			UpdateFromKeyboard();
		}
		break;

		case eGameState::PLAYING:
		{
			if ( !m_hasInitialDialogueHappened )
			{
				// Initial dialogue
				m_player->SetPosition( Vec2( 25.f, m_player->GetPosition().y ) );
				m_player->SetMoveTargetLocation( Vec2( 25.f, m_player->GetPosition().y ) );
				DialogueState* initialState = DialogueState::GetDialogueState( "PurpleTentacle_initial" );
				BeginConversation( initialState, m_world->GetPurpleTentacle() );

				m_hasInitialDialogueHappened = true;
				return;
			}

			UpdateFromKeyboard();

			if ( m_verbText.empty() )
			{
				m_verbActionUIText->SetText( "Walk to " );
			}
			else
			{
				std::string actionText( m_verbText + " " + m_nounText );

				if ( m_player->GetPlayerVerbState() == eVerbState::GIVE_TO_DESTINATION )
				{
					actionText += " to ";
					actionText += m_giveTargetNounText;
				}

				m_verbActionUIText->SetText( actionText );
			}

			m_world->Update();
		}
		break;

		case eGameState::DIALOGUE:
		{
			UpdateFromKeyboard();
			UpdateNPCResponse();

			m_world->Update();
		}
		break;
	}

	UpdateCameras();
	UpdateMousePositions();

	m_uiSystem->Update();

	if ( m_cursor != nullptr )
	{
		m_cursor->Update( (float)m_gameClock->GetLastDeltaSeconds() );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_gameState )
	{
		case eGameState::PLAYING:
		case eGameState::PAUSED:
		case eGameState::DIALOGUE:
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
	}

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

	if ( m_cursor != nullptr )
	{
		m_cursor->Render();
	}

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Game::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
void Game::StopAllSounds()
{
	if ( m_attractMusicID != -1 )
	{
		g_audioSystem->StopSound( m_attractMusicID );
	}
	if ( m_gameplayMusicID != -1 )
	{
		g_audioSystem->StopSound( m_gameplayMusicID );
	}
	if ( m_victoryMusicID != -1 )
	{
		g_audioSystem->StopSound( m_victoryMusicID );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::ResetGame()
{
	m_world->UnloadCurrentMap();
	
	PTR_SAFE_DELETE( m_cursor );
	PTR_SAFE_DELETE( m_player );

	LoadEntitiesFromXml();
	LoadMapsFromXml();

	m_inventory.clear();
	UpdateInventoryButtonImages();

	m_hasInitialDialogueHappened = false;
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );

	// Music
	g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheScummBar.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheLookout.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Music/MeleeIsland.mp3" );

	// TODO: Check for nullptrs when loading textures
	//g_tileSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_32x32.png" )), IntVec2( 32, 32 ) );
	//g_characterSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaCharacters_12x53.png" )), IntVec2( 12, 53 ) );
	//g_portraitSpriteSheet = new SpriteSheet( *(g_renderer->CreateOrGetTextureFromFile( "Data/Images/KushnariovaPortraits_8x8.png" )), IntVec2( 8, 8 ) );

	g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" );
	g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/DebugRender.hlsl" );

	LoadDialogueStatesFromXml();
	LoadEntitiesFromXml();
	LoadMapsFromXml();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadDialogueStatesFromXml()
{
	g_devConsole->PrintString( "Loading Dialogue States..." );

	const char* filePath = "Data/Gameplay/DialogueStateDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The dialogue state xml file '%s' could not be opened.", filePath ) );
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		DialogueState* diagState = new DialogueState( *element );
		if ( diagState->IsValid() )
		{
			DialogueState::s_dialogueStateMap[diagState->GetName()] = diagState;
		}

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Dialogue States Loaded", Rgba8::GREEN );
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

		m_world->LoadMap( mapDef->GetName() );

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadEntitiesFromXml()
{
	g_devConsole->PrintString( "Loading Entities..." );

	const char* filePath = "Data/Gameplay/EntityDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The entities xml file '%s' could not be opened.", filePath ) );
	}

	XmlElement* root = doc.RootElement();
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		EntityDefinition* entityDef = new EntityDefinition( *element );
		EntityDefinition::s_definitions[entityDef->GetName()] = entityDef;
		
		if ( entityDef->GetName() == "Player" )
		{
			m_player = new Actor( Vec2::ZERO, entityDef );
		}
		else if ( entityDef->GetName() == "Cursor" )
		{
			m_cursor = new Cursor( Vec2::ZERO, entityDef );
		}

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Entities Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}

	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_eventSystem->FireEvent( "Quit" );
			}

			m_mainMenuPanel->Update();
			g_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON );
		}
		break;

		case eGameState::VICTORY:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_eventSystem->FireEvent( "Quit" );
			}

			m_victoryPanel->Update();
			g_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON );
		}
		break;

		case eGameState::PLAYING:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				ChangeGameState( eGameState::PAUSED );
			}
			
			if ( m_player != nullptr )
			{
				if ( g_inputSystem->WasKeyJustPressed( MOUSE_RBUTTON ) )
				{
					// Check if click was in hud
					if ( GetMouseWorldPosition().y < 0.25f )
					{
						return;
					}

					m_player->SetMoveTargetLocation( GetMouseWorldPosition() );
				}
				if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
				{
					// Check if click was in hud
					if ( GetMouseWorldPosition().y < 0.25f )
					{
						return;
					}

					// Click was in world, consume all key presses for click
					g_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON );

					if ( !m_player->IsExecutingAction() )
					{
						if ( m_player->GetPlayerVerbState() == eVerbState::GIVE_TO_DESTINATION )
						{
							if ( m_giveTargetNounText != "" )
							{
								m_player->ExecuteAction( m_giveTargetNounText );
							}
							else
							{
								m_player->StopExecutingAction();
							}
						}
						else if ( m_nounText != "" )
						{
							m_player->ExecuteAction( m_nounText );
						}
					}
					else
					{
						m_player->StopExecutingAction();
					}
					
					m_player->SetMoveTargetLocation( GetMouseWorldPosition() );
				}
			}
		}
		break;

		case eGameState::PAUSED:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				ChangeGameState( eGameState::PLAYING );
			}
		}
		break;

		case eGameState::DIALOGUE:
		{
			if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
			{
				if ( GetMouseWorldPosition().y > 0.25f )
				{
					// Click was in world, consume all key presses for click
					g_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON );
				}
			}
		}
		break;
	}
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
void Game::UpdateNPCResponse()
{
	if ( m_dialogueNPC == nullptr )
	{
		return;
	}

	Vec2 textPosition( m_dialogueNPC->GetPosition() );
	textPosition.y += 1.f;

	DebugAddWorldTextf( Mat44::CreateTranslation2D( textPosition ),
						Vec2( .5f, .5f ),
						Rgba8::WHITE,
						0.f,
						0.14f,
						DEBUG_RENDER_ALWAYS,
						m_dialogueNPCText.c_str() );
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
			if ( m_gameState == eGameState::VICTORY )
			{
				m_victoryPanel->Deactivate();
				m_victoryPanel->Hide();

				g_audioSystem->StopSound( m_victoryMusicID );
			}

			m_hudPanel->Deactivate();
			m_hudPanel->Hide();

			m_mainMenuPanel->Activate();
			m_mainMenuPanel->Show();

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

			/*SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheLookout.mp3" );
			m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true, .25f );*/
		}
		break;

		case eGameState::PLAYING:
		{
			m_dialoguePanel->Deactivate();
			m_dialoguePanel->Hide();
			m_hudPanel->Activate();
			m_hudPanel->Show();

			// Check which state we are changing from
			switch ( m_gameState )
			{
				case eGameState::PAUSED:
				{
					/*SoundID unpause = g_audioSystem->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
					g_audioSystem->PlaySound( unpause );*/

					g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, .25f );

					m_hudPanel->Activate();
					m_pauseMenuPanel->Hide();
					m_pauseMenuPanel->Deactivate();
				}
				break;

				case eGameState::ATTRACT:
				{
					m_mainMenuPanel->Deactivate();
					m_mainMenuPanel->Hide();
					g_audioSystem->StopSound( m_attractMusicID );

					SoundID gameplayMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheScummBar.mp3" );
					m_gameplayMusicID = g_audioSystem->PlaySound( gameplayMusic, true, .25f );
					
					m_curMap = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMap );
					g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMap.c_str() ) );
					ChangeMap( m_curMap );
				}
				break;
			}
		}
		break;

		case eGameState::DIALOGUE:
		{
			switch ( m_gameState )
			{
				case eGameState::PLAYING:
				{
					m_hudPanel->Deactivate();
					m_hudPanel->Hide();
					m_dialoguePanel->Activate();
					m_dialoguePanel->Show();
					//g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, false, true );
				}
			}
		}
		break;

		case eGameState::PAUSED:
		{
			g_audioSystem->SetSoundPlaybackVolume( m_gameplayMusicID, .1f );

			/*SoundID pause = g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
			g_audioSystem->PlaySound( pause );*/

			m_hudPanel->Deactivate();
			m_pauseMenuPanel->Activate();
			m_pauseMenuPanel->Show();

		}
		break;

		case eGameState::VICTORY:
		{
			m_hudPanel->Deactivate();
			m_hudPanel->Hide();
			m_dialoguePanel->Deactivate();
			m_dialoguePanel->Hide();

			m_victoryPanel->Show();
			m_victoryPanel->Activate();

			g_audioSystem->StopSound( m_gameplayMusicID );

			SoundID victoryMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/MeleeIsland.mp3" );
			m_victoryMusicID = g_audioSystem->PlaySound( victoryMusic, true, .25f );
		}
		break;
	}

	m_gameState = newGameState;
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMap( const std::string& mapName )
{
	m_world->ChangeMap( mapName, m_player, m_cursor );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildMenus()
{
	Rgba8 tint = Rgba8::WHITE;
	tint.r -= 30;
	tint.g -= 30;
	tint.b -= 30;

	UIAlignedPositionData buttonTextPositionCentered;
	buttonTextPositionCentered.fractionOfParentDimensions = Vec2::ONE;
	buttonTextPositionCentered.alignmentWithinParentElement = ALIGN_CENTERED;

	BuildMainMenu( tint, buttonTextPositionCentered );
	BuildPauseMenu( tint, buttonTextPositionCentered );
	BuildVictoryMenu( tint, buttonTextPositionCentered );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildMainMenu( const Rgba8& tint, const UIAlignedPositionData& textPositionData )
{
	UIRelativePositionData panelPositionData;
	panelPositionData.widthFractionRange = Vec2::ZERO_TO_ONE;
	panelPositionData.heightFractionRange = Vec2::ZERO_TO_ONE;

	m_mainMenuPanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/MainMenuBackground.png" ) );

	UIAlignedPositionData logoPositionData;
	logoPositionData.fractionOfParentDimensions = Vec2( .7f, .7f );
	logoPositionData.alignmentWithinParentElement = ALIGN_TOP_CENTER;

	m_mainMenuPanel->AddChildPanel( logoPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/TheTentacleOfMonkeyIsland-logo.png" ) );

	UIAlignedPositionData buttonPositionData;
	buttonPositionData.fractionOfParentDimensions = Vec2( .1f, .05f );
	buttonPositionData.positionOffsetFraction = Vec2( .0f, -.33f );
	buttonPositionData.alignmentWithinParentElement = ALIGN_CENTERED;

	m_mainMenuPlayButton = m_mainMenuPanel->AddButton( buttonPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/UIButtonBackground.png" ) );
	m_mainMenuPlayButton->AddText( textPositionData, "Play" );
	m_mainMenuPlayButton->SetButtonAndLabelTint( tint );
	m_mainMenuPlayButton->m_onClickEvent.SubscribeMethod( this, &Game::OnMainMenuPlayButtonClicked );
	m_mainMenuPlayButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverBegin );
	m_mainMenuPlayButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverEnd );

	buttonPositionData.positionOffsetFraction = Vec2( .0f, -.43f );
	m_mainMenuExitButton = m_mainMenuPanel->AddButton( buttonPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/UIButtonBackground.png" ) );
	m_mainMenuExitButton->AddText( textPositionData, "Quit" );
	m_mainMenuExitButton->SetButtonAndLabelTint( tint );
	m_mainMenuExitButton->m_onClickEvent.SubscribeMethod( this, &Game::OnMainMenuExitButtonClicked );
	m_mainMenuExitButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverBegin );
	m_mainMenuExitButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverEnd );

	m_mainMenuPanel->Deactivate();
	m_mainMenuPanel->Hide();
}


//-----------------------------------------------------------------------------------------------
void Game::BuildPauseMenu( const Rgba8& tint, const UIAlignedPositionData& textPositionData )
{
	UIAlignedPositionData panelPositionData;
	panelPositionData.fractionOfParentDimensions = Vec2( .35f, .65f );
	
	m_pauseMenuPanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/MainMenuBackground.png" ) );

	panelPositionData.fractionOfParentDimensions = Vec2( .7f, .7f );
	UIPanel* titlePanel = m_pauseMenuPanel->AddChildPanel( panelPositionData, nullptr );
	titlePanel->AddText( textPositionData, "Paused", 48.f );

	UIAlignedPositionData buttonPositionData;
	buttonPositionData.fractionOfParentDimensions = Vec2( .3f, .1f );
	buttonPositionData.positionOffsetFraction = Vec2( .0f, -.25f );
	buttonPositionData.alignmentWithinParentElement = ALIGN_CENTERED;

	m_pauseMenuResumeButton = m_pauseMenuPanel->AddButton( buttonPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/UIButtonBackground.png" ) );
	m_pauseMenuResumeButton->AddText( textPositionData, "Resume" );
	m_pauseMenuResumeButton->SetButtonAndLabelTint( tint );
	m_pauseMenuResumeButton->m_onClickEvent.SubscribeMethod( this, &Game::OnPauseMenuResumeButtonClicked );
	m_pauseMenuResumeButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverBegin );
	m_pauseMenuResumeButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverEnd );

	buttonPositionData.positionOffsetFraction = Vec2( .0f, -.42f );

	m_pauseMenuExitButton = m_pauseMenuPanel->AddButton( buttonPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/UIButtonBackground.png" ) );
	m_pauseMenuExitButton->AddText( textPositionData, "Quit" );
	m_pauseMenuExitButton->SetButtonAndLabelTint( tint );
	m_pauseMenuExitButton->m_onClickEvent.SubscribeMethod( this, &Game::OnPauseMenuExitButtonClicked );
	m_pauseMenuExitButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverBegin );
	m_pauseMenuExitButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverEnd );

	m_pauseMenuPanel->Deactivate();
	m_pauseMenuPanel->Hide();
}


//-----------------------------------------------------------------------------------------------
void Game::BuildVictoryMenu( const Rgba8& tint, const UIAlignedPositionData& textPositionData )
{
	UIRelativePositionData panelPositionData;
	panelPositionData.widthFractionRange = Vec2::ZERO_TO_ONE;
	panelPositionData.heightFractionRange = Vec2::ZERO_TO_ONE;

	m_victoryPanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/VictoryScreen.png" ) );

	UIAlignedPositionData buttonPositionData;
	buttonPositionData.fractionOfParentDimensions = Vec2( .1f, .05f );
	buttonPositionData.positionOffsetFraction = Vec2( .0f, -.33f );
	buttonPositionData.alignmentWithinParentElement = ALIGN_CENTERED;

	m_victoryRetryButton = m_victoryPanel->AddButton( buttonPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/UIButtonBackground.png" ) );
	m_victoryRetryButton->AddText( textPositionData, "Retry" );
	m_victoryRetryButton->SetButtonAndLabelTint( tint );
	m_victoryRetryButton->m_onClickEvent.SubscribeMethod( this, &Game::OnPauseMenuExitButtonClicked );
	m_victoryRetryButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverBegin );
	m_victoryRetryButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverEnd );

	buttonPositionData.positionOffsetFraction = Vec2( .0f, -.4f );
	m_victoryExitButton = m_victoryPanel->AddButton( buttonPositionData, g_renderer->CreateOrGetTextureFromFile( "Data/Images/UIButtonBackground.png" ) );
	m_victoryExitButton->AddText( textPositionData, "Quit" );
	m_victoryExitButton->SetButtonAndLabelTint( tint );
	m_victoryExitButton->m_onClickEvent.SubscribeMethod( this, &Game::OnMainMenuExitButtonClicked );
	m_victoryExitButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverBegin );
	m_victoryExitButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnMenuButtonHoverEnd );

	m_victoryPanel->Deactivate();
	m_victoryPanel->Hide();
}


//-----------------------------------------------------------------------------------------------
void Game::BuildHUD()
{
	Texture* childBackground = g_renderer->GetDefaultWhiteTexture();

	UIAlignedPositionData panelPositionData;
	panelPositionData.fractionOfParentDimensions = Vec2( 1.f, .25f );
	panelPositionData.alignmentWithinParentElement = ALIGN_BOTTOM_CENTER;

	m_hudPanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPositionData, childBackground, Rgba8::BLACK );
	m_dialoguePanel = m_uiSystem->GetRootPanel()->AddChildPanel( panelPositionData, childBackground, Rgba8::BLACK );

	panelPositionData.fractionOfParentDimensions = Vec2( 1.f, .1f );
	panelPositionData.alignmentWithinParentElement = ALIGN_TOP_CENTER;
	m_currentActionPanel = m_hudPanel->AddChildPanel( panelPositionData, childBackground, Rgba8::BLACK );

	UIAlignedPositionData textPositionData;
	textPositionData.fractionOfParentDimensions = Vec2( 1.f, 1.f );
	textPositionData.verticalPaddingInPixels = Vec2( 2.f, 2.f );

	m_verbActionUIText = (UIText*)m_currentActionPanel->AddText( textPositionData, "" );

	BuildVerbPanel();
	BuildInventoryPanel();

	m_hudPanel->Deactivate();
	m_hudPanel->Hide();
	m_dialoguePanel->Deactivate();
	m_dialoguePanel->Hide();
}


//-----------------------------------------------------------------------------------------------
void Game::BuildVerbPanel()
{
	Texture* background = g_renderer->GetDefaultWhiteTexture();
	
	UIAlignedPositionData textPositionData;
	textPositionData.fractionOfParentDimensions = Vec2( .8f, .8f );

	UIAlignedPositionData panelPositionData;
	panelPositionData.fractionOfParentDimensions = Vec2( .5f, .9f );
	panelPositionData.alignmentWithinParentElement = ALIGN_BOTTOM_LEFT;

	m_verbPanel = m_hudPanel->AddChildPanel( panelPositionData, background, Rgba8::BLACK );

	// Top Row
	UIAlignedPositionData buttonPositionData;
	buttonPositionData.fractionOfParentDimensions = Vec2( .333f, .5f );
	buttonPositionData.horizontalPaddingInPixels = Vec2( 5.f, 5.f );
	buttonPositionData.verticalPaddingInPixels = Vec2( 5.f, 0.f );

	buttonPositionData.alignmentWithinParentElement = ALIGN_TOP_LEFT;
	m_giveVerbButton = m_verbPanel->AddButton( buttonPositionData, background, Rgba8::DARK_BLUE );
	m_giveVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnVerbButtonClicked );
	m_giveVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_giveVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_giveVerbButton->AddText( textPositionData, "Give" );

	buttonPositionData.alignmentWithinParentElement = ALIGN_TOP_CENTER;
	m_openVerbButton = m_verbPanel->AddButton( buttonPositionData, background, Rgba8::DARK_BLUE );
	m_openVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnVerbButtonClicked );
	m_openVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_openVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_openVerbButton->AddText( textPositionData, "Open" );

	buttonPositionData.alignmentWithinParentElement = ALIGN_TOP_RIGHT;
	m_closeVerbButton = m_verbPanel->AddButton( buttonPositionData, background, Rgba8::DARK_BLUE );
	m_closeVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnVerbButtonClicked );
	m_closeVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_closeVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_closeVerbButton->AddText( textPositionData, "Close" );

	// Bottom Row
	buttonPositionData.fractionOfParentDimensions = Vec2( .5f, .5f );
	buttonPositionData.alignmentWithinParentElement = ALIGN_BOTTOM_LEFT;
	m_pickUpVerbButton = m_verbPanel->AddButton( buttonPositionData, background, Rgba8::DARK_BLUE );
	m_pickUpVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnVerbButtonClicked );
	m_pickUpVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_pickUpVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_pickUpVerbButton->AddText( textPositionData, "Pick up" );

	buttonPositionData.alignmentWithinParentElement = ALIGN_BOTTOM_RIGHT;
	m_talkToVerbButton = m_verbPanel->AddButton( buttonPositionData, background, Rgba8::DARK_BLUE );
	m_talkToVerbButton->m_onClickEvent.SubscribeMethod( this, &Game::OnVerbButtonClicked );
	m_talkToVerbButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
	m_talkToVerbButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
	m_talkToVerbButton->AddText( textPositionData, "Talk to" );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildInventoryPanel()
{
	Texture* background = g_renderer->GetDefaultWhiteTexture();

	UIAlignedPositionData panelPositionData;
	panelPositionData.fractionOfParentDimensions = Vec2( .5f, .9f );
	panelPositionData.alignmentWithinParentElement = ALIGN_BOTTOM_RIGHT;

	m_inventoryPanel = m_hudPanel->AddChildPanel( panelPositionData, background, Rgba8::BLACK );

	constexpr int NUM_IN_ROW = 4;
	constexpr int NUM_ROWS = 2;
	constexpr float rowHeightFraction = 1.f / (float)NUM_ROWS;
	constexpr float rowWidthFraction = 1.f / (float)NUM_IN_ROW;

	UIAlignedPositionData buttonPositionData;
	buttonPositionData.fractionOfParentDimensions = Vec2( rowWidthFraction, rowHeightFraction );
	buttonPositionData.positionOffsetFraction = Vec2::ZERO;
	buttonPositionData.horizontalPaddingInPixels = Vec2( 2.5f, 2.5f );
	buttonPositionData.verticalPaddingInPixels = Vec2( 5.f, 0.f );
	buttonPositionData.alignmentWithinParentElement = ALIGN_TOP_LEFT;
	
	for ( int buttonRowNum = 0; buttonRowNum < NUM_ROWS; ++buttonRowNum )
	{
		for ( int buttonColumnNum = 0; buttonColumnNum < NUM_IN_ROW; ++buttonColumnNum )
		{
			UIButton* inventoryButton = m_inventoryPanel->AddButton( buttonPositionData, background, Rgba8::DARK_BLUE );
			inventoryButton->m_onClickEvent.SubscribeMethod( this, &Game::OnInventoryButtonClicked );
			inventoryButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnTestButtonHoverBegin );
			inventoryButton->m_onHoverStayEvent.SubscribeMethod( this, &Game::OnInventoryItemHoverStay );
			inventoryButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnTestButtonHoverEnd );
			
			m_inventoryButtons.push_back( inventoryButton );

			buttonPositionData.positionOffsetFraction.x += rowWidthFraction;
		}

		buttonPositionData.positionOffsetFraction = Vec2( 0.f, -rowHeightFraction );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::AddDialogueOptionsToHUD( const std::vector<std::string>& dialogueChoices, float fontSize )
{
	int numChoices = (int)dialogueChoices.size();
	if ( numChoices == 0 )
	{
		return;
	}

	float choiceHeight = fontSize / m_dialoguePanel->GetBoundingBox().GetHeight();
	float currentHeight = 1.f - choiceHeight;
	float spacing = choiceHeight * .5f;
	for ( int choiceIdx = 0; choiceIdx < numChoices; ++choiceIdx )
	{
		UIRelativePositionData buttonPositionData;
		buttonPositionData.widthFractionRange = Vec2( 0.f, 1.f );
		buttonPositionData.heightFractionRange = Vec2( currentHeight - choiceHeight, choiceHeight );
		buttonPositionData.horizontalPaddingInPixels = Vec2( 3.f, 0.f );

		UIButton* newButton = m_dialoguePanel->AddButton( buttonPositionData );
		UIRelativePositionData textPositionData;
		newButton->AddText( textPositionData, dialogueChoices[choiceIdx], fontSize, ALIGN_CENTERED_LEFT );

		newButton->m_onClickEvent.SubscribeMethod( this, &Game::OnDialogueChoiceClicked );
		newButton->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnDialogueChoiceHoverBegin );
		newButton->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnDialogueChoiceHoverEnd );

		NamedProperties* userData = new NamedProperties();
		userData->SetValue( "choiceText", dialogueChoices[choiceIdx] );
		newButton->SetUserData( userData );

		currentHeight -= ( choiceHeight + spacing );
	}
}


//-----------------------------------------------------------------------------------------------
// Button events
//-----------------------------------------------------------------------------------------------
void Game::OnMainMenuPlayButtonClicked( EventArgs* args )
{
	UNUSED( args );
	ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
void Game::OnMainMenuExitButtonClicked( EventArgs* args )
{
	UNUSED( args );
	g_eventSystem->FireEvent( "Quit" );
}


//-----------------------------------------------------------------------------------------------
void Game::OnPauseMenuResumeButtonClicked( EventArgs* args )
{
	UNUSED( args );
	ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
void Game::OnPauseMenuExitButtonClicked( EventArgs* args )
{
	UNUSED( args );

	m_pauseMenuPanel->Deactivate();
	m_pauseMenuPanel->Hide();

	ResetGame();
	
	SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Music/TheLookout.mp3" );
	m_attractMusicID = g_audioSystem->PlaySound( attractMusic, true, .25f );
	
	ChangeGameState( eGameState::ATTRACT );
}


//-----------------------------------------------------------------------------------------------
void Game::OnVerbButtonClicked( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );
	
	eVerbState verbState = eVerbState::NONE;
	//if ( id == m_giveVerbButton->GetId() ) { m_giveVerbButton->SetTint( tint );	return; }
	if ( id == m_openVerbButton->GetId() ) { verbState = eVerbState::OPEN; }
	else if ( id == m_closeVerbButton->GetId() ) { verbState = eVerbState::CLOSE; }
	else if ( id == m_pickUpVerbButton->GetId() ) { verbState = eVerbState::PICKUP; }
	else if ( id == m_talkToVerbButton->GetId() ) { verbState = eVerbState::TALK_TO; }
	else if ( id == m_giveVerbButton->GetId() ) { verbState = eVerbState::GIVE_TO_SOURCE; }

	m_player->StopExecutingAction();
	m_player->SetPlayerVerbState( verbState );
	m_verbText = GetDisplayNameForVerbState( verbState );
	m_nounText = "";
}


//-----------------------------------------------------------------------------------------------
void Game::OnInventoryButtonClicked( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );

	if ( m_player->GetPlayerVerbState() == eVerbState::NONE )
	{
		return;
	}

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			std::string itemName = m_inventory[inventoryButtonIdx]->GetName();

			m_inventory[inventoryButtonIdx]->HandleVerbAction( m_player->GetPlayerVerbState() );
			
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnTestButtonClicked( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );

	g_devConsole->PrintString( "Button clicked!", Rgba8::ORANGE );

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{

			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnInventoryItemHoverStay( EventArgs* args )
{
	if ( m_player->IsExecutingAction() )
	{
		return;
	}

	uint id = args->GetValue( "id", (uint)0 );

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			std::string itemName = m_inventory[inventoryButtonIdx]->GetName();
			if ( m_player->GetPlayerVerbState() == eVerbState::GIVE_TO_DESTINATION )
			{
				m_giveTargetNounText = itemName;
			}
			else
			{
				m_nounText = itemName;
			}

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

	if ( id == m_giveVerbButton->GetId() )	 { m_giveVerbButton->SetButtonTint( tint );	return; }
	if ( id == m_openVerbButton->GetId() )	 { m_openVerbButton->SetButtonTint( tint );	return; }
	if ( id == m_closeVerbButton->GetId() )  { m_closeVerbButton->SetButtonTint( tint );	return; }
	if ( id == m_pickUpVerbButton->GetId() ) { m_pickUpVerbButton->SetButtonTint( tint ); return; }
	if ( id == m_talkToVerbButton->GetId() ) { m_talkToVerbButton->SetButtonTint( tint ); return; }

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			itemButton->SetButtonTint( tint );
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnTestButtonHoverEnd( EventArgs* args )
{
	uint id = args->GetValue( "id", (uint)0 );

	Rgba8 tint = Rgba8::DARK_BLUE;

	if ( id == m_giveVerbButton->GetId() ) { m_giveVerbButton->SetButtonTint( tint );	return; }
	if ( id == m_openVerbButton->GetId() ) { m_openVerbButton->SetButtonTint( tint );	return; }
	if ( id == m_closeVerbButton->GetId() ) { m_closeVerbButton->SetButtonTint( tint );	return; }
	if ( id == m_pickUpVerbButton->GetId() ) { m_pickUpVerbButton->SetButtonTint( tint ); return; }
	if ( id == m_talkToVerbButton->GetId() ) { m_talkToVerbButton->SetButtonTint( tint ); return; }

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		UIButton*& itemButton = m_inventoryButtons[inventoryButtonIdx];
		if ( id == itemButton->GetId() )
		{
			itemButton->SetButtonTint( tint );

			if ( !m_player->IsExecutingAction()
				&& m_player->GetPlayerVerbState() == eVerbState::GIVE_TO_DESTINATION )
			{
				m_giveTargetNounText = "";
			}

			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnMenuButtonHoverBegin( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}
	
	button->SetButtonAndLabelTint( Rgba8::WHITE );
}


//-----------------------------------------------------------------------------------------------
void Game::OnMenuButtonHoverEnd( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	Rgba8 tint = Rgba8::WHITE;
	tint.r -= 30;
	tint.g -= 30;
	tint.b -= 30;

	button->SetButtonAndLabelTint( tint );
}


//-----------------------------------------------------------------------------------------------
void Game::OnDialogueChoiceClicked( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	if ( button->GetUserData() == nullptr )
	{
		return;
	}

	std::string choiceText = button->GetUserData()->GetValue( "choiceText", "" );

	DialogueState* nextState = m_curDialogueState->GetNextDialogueStateFromChoice( choiceText );
	ChangeDialogueState( nextState );
}


//-----------------------------------------------------------------------------------------------
void Game::OnDialogueChoiceHoverBegin( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", (void*)nullptr );
	if ( button == nullptr )
	{
		return;
	}

	button->SetButtonAndLabelTint( Rgba8::YELLOW );
}


//-----------------------------------------------------------------------------------------------
void Game::OnDialogueChoiceHoverEnd( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	button->SetButtonAndLabelTint( Rgba8::WHITE );
}


//-----------------------------------------------------------------------------------------------
void Game::AddItemToInventory( Entity* newItem )
{
	NamedProperties* properties = new NamedProperties();
	properties->SetValue( "eventName", GetEventNameForVerbState( eVerbState::GIVE_TO_SOURCE ) );
	newItem->AddVerbState( eVerbState::GIVE_TO_SOURCE, properties );

	m_inventory.push_back( newItem );
	newItem->SetIsInPlayerInventory( true );

	UpdateInventoryButtonImages();
}


//-----------------------------------------------------------------------------------------------
void Game::RemoveItemFromInventory( Entity* itemToRemove )
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
void Game::RemoveItemFromInventory( const std::string& itemName )
{
	int itemIdx = 0;
	for ( ; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx]->GetName() == itemName )
		{
			m_inventory[itemIdx]->SetIsInPlayerInventory( false );
			break;
		}
	}

	if ( itemIdx < (int)m_inventory.size() )
	{
		m_inventory.erase( m_inventory.begin() + itemIdx );

		UpdateInventoryButtonImages();
	}
}


//-----------------------------------------------------------------------------------------------
bool Game::IsItemInInventory( Entity* item )
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
bool Game::IsItemInInventory( const std::string& itemName )
{
	// consider a null item name as a success
	if ( itemName == "" )
	{
		return true;
	}

	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx]->GetName() == itemName )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Game::SetPlayerVerbState( eVerbState verbState )
{
	if ( m_player != nullptr )
	{
		m_player->SetPlayerVerbState( verbState );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::ClearCurrentActionText()
{
	m_verbText = "";
	m_nounText = "";
}


//-----------------------------------------------------------------------------------------------
void Game::SetNounText( const std::string& nounText )
{
	if ( m_player->GetPlayerVerbState() == eVerbState::GIVE_TO_DESTINATION )
	{
		m_giveTargetNounText = nounText;
	}
	else
	{
		m_nounText = nounText;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::PrintTextOverPlayer( const std::string& text )
{
	Vec2 textPosition( m_player->GetPosition() );
	textPosition.y += 1.f;

	DebugAddWorldTextf( Mat44::CreateTranslation2D( textPosition ),
						Vec2( .5f, .5f ),
						Rgba8::WHITE,
						1.f,
						0.15f,
						DEBUG_RENDER_ALWAYS,
						text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void Game::PrintTextOverEntity( const Entity& entity, const std::string& text, float duration )
{
	Vec2 textPosition( entity.GetPosition() );
	textPosition.y += 1.f;

	DebugAddWorldTextf( Mat44::CreateTranslation2D( textPosition ),
						Vec2( .5f, .5f ),
						Rgba8::WHITE,
						duration,
						0.15f,
						DEBUG_RENDER_ALWAYS,
						text.c_str() );

	m_dialogueTimer.SetSeconds( m_gameClock, (double)duration );
}


//-----------------------------------------------------------------------------------------------
void Game::BeginConversation( DialogueState* initialDialogueState, Entity* dialoguePartner )
{
	ChangeGameState( eGameState::DIALOGUE );
	m_dialogueNPC = dialoguePartner;
	if ( m_dialogueNPC->GetName() == "Purple Tentacle"
		 && IsItemInInventory( "Thick Book of Maps" )
		 && IsItemInInventory( "Red Herring" ) 
		 && IsItemInInventory( "Wooden Arms" ) )
	{
		ChangeGameState( eGameState::VICTORY );
		return;
	}

	ChangeDialogueState( initialDialogueState );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeDialogueState( DialogueState* newDialogueState )
{
	m_curDialogueState = newDialogueState;
	if ( newDialogueState == nullptr )
	{
		EndConversation();
		return;
	}

	if ( m_dialogueNPC != nullptr )
	{
		m_dialogueNPCText = newDialogueState->GetIntroText();
	}

	if ( newDialogueState->GetItemName() != "" )
	{
		if ( g_game->IsItemInInventory( newDialogueState->GetItemName() ) )
		{

		}
		else
		{
			EntityDefinition* itemDef = EntityDefinition::GetEntityDefinition( newDialogueState->GetItemName() );
			if ( itemDef != nullptr )
			{
				AddItemToInventory( new Entity( Vec2::ZERO, itemDef ) );
			}
		}
	}

	if ( newDialogueState->GetDialogueChoices().size() == 0 )
	{
		PrintTextOverEntity( *m_dialogueNPC, newDialogueState->GetIntroText(), 2.f );
		EndConversation();
		return;
	}

	m_dialoguePanel->ClearButtons();
	AddDialogueOptionsToHUD( newDialogueState->GetDialogueChoices(), 24.f );
}


//-----------------------------------------------------------------------------------------------
void Game::EndConversation()
{
	m_dialogueNPC = nullptr;
	ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateInventoryButtonImages()
{
	UIAlignedPositionData positionData;
	positionData.fractionOfParentDimensions = Vec2( .8f, .8f );

	for ( int inventoryButtonIdx = 0; inventoryButtonIdx < (int)m_inventoryButtons.size(); ++inventoryButtonIdx )
	{
		// Reset button
		m_inventoryButtons[inventoryButtonIdx]->ClearLabels();
		m_inventoryButtons[inventoryButtonIdx]->Deactivate();

		if ( inventoryButtonIdx < (int)m_inventory.size() )
		{
			SpriteDefinition* spriteDef = m_inventory[inventoryButtonIdx]->GetSpriteDef();

			m_inventoryButtons[inventoryButtonIdx]->AddImage( positionData, spriteDef );
			m_inventoryButtons[inventoryButtonIdx]->Activate();
		}
	}

}
