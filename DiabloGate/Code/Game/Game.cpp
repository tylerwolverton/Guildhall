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
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIUniformGrid.hpp"
#include "Engine/UI/UIPanel.hpp"

#include "Game/Actor.hpp"
#include "Game/Entity.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapData.hpp"
#include "Game/LevelThresholdDefinition.hpp"
#include "Game/Scripting/ZephyrCompiler.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
GameTimer::GameTimer( Clock* clock, const EntityId& targetId, const std::string& callbackName, const std::string& name, EventArgs* callbackArgsIn )
	: targetId( targetId )
	, name( name )
	, callbackName( callbackName )
{
	timer = Timer( clock );

	callbackArgs = new EventArgs();
	for ( auto const& keyValuePair : callbackArgsIn->GetAllKeyValuePairs() )
	{
		if ( keyValuePair.second->Is<float>() )
		{
			callbackArgs->SetValue( keyValuePair.first, callbackArgsIn->GetValue( keyValuePair.first, 0.f ) );
		}
		else if ( keyValuePair.second->Is<int>() )
		{
			callbackArgs->SetValue( keyValuePair.first, callbackArgsIn->GetValue( keyValuePair.first, (EntityId)-1 ) );
		}
		else if ( keyValuePair.second->Is<double>() )
		{
			callbackArgs->SetValue( keyValuePair.first, (float)callbackArgsIn->GetValue( keyValuePair.first, 0.0 ) );
		}
		else if ( keyValuePair.second->Is<bool>() )
		{
			callbackArgs->SetValue( keyValuePair.first, callbackArgsIn->GetValue( keyValuePair.first, false ) );
		}
		else if ( keyValuePair.second->Is<Vec2>() )
		{
			callbackArgs->SetValue( keyValuePair.first, callbackArgsIn->GetValue( keyValuePair.first, Vec2::ZERO ) );
		}
		else if ( keyValuePair.second->Is<std::string>()
				  || keyValuePair.second->Is<char*>() )
		{
			callbackArgs->SetValue( keyValuePair.first, callbackArgsIn->GetValue( keyValuePair.first, "" ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
GameTimer::~GameTimer()
{
	PTR_SAFE_DELETE( callbackArgs );
}


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
	
	InitializeFPSHistory();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	Clock::GetMaster()->SetFrameLimits( 1.0 / 60.0, .1 );

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );
	g_physicsSystem2D->Startup( m_gameClock );
	g_physicsSystem2D->SetSceneGravity( 0.f );

	g_physicsSystem2D->DisableAllLayerInteraction( eCollisionLayer::NONE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::STATIC_ENVIRONMENT, eCollisionLayer::STATIC_ENVIRONMENT );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::PLAYER, eCollisionLayer::PLAYER_PROJECTILE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::ENEMY, eCollisionLayer::ENEMY_PROJECTILE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::PLAYER_PROJECTILE, eCollisionLayer::PLAYER_PROJECTILE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::ENEMY_PROJECTILE, eCollisionLayer::ENEMY_PROJECTILE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::PORTAL, eCollisionLayer::PLAYER_PROJECTILE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::PORTAL, eCollisionLayer::ENEMY_PROJECTILE );
	g_physicsSystem2D->DisableLayerInteraction( eCollisionLayer::PORTAL, eCollisionLayer::STATIC_ENVIRONMENT );
	g_physicsSystem2D->DisableAllLayerInteraction( eCollisionLayer::PICKUP );
	g_physicsSystem2D->EnableLayerInteraction( eCollisionLayer::PLAYER, eCollisionLayer::PICKUP );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	m_uiFont = g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/FiraCode", true );
	m_uiFontMaterial = new Material( g_renderer, "Data/Materials/Font.material" );
	
	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer, m_uiFont, m_uiFontMaterial );

	m_world = new World( m_gameClock );

	m_gameName = g_gameConfigBlackboard.GetValue( "windowTitle", m_gameName );
	m_cameraSpeed = g_gameConfigBlackboard.GetValue( "cameraSpeed", m_cameraSpeed );
	m_maxMasterVolume = g_gameConfigBlackboard.GetValue( "masterVolume", m_maxMasterVolume );
	m_startingMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_startingMapName );
	m_dataPathSuffix = g_gameConfigBlackboard.GetValue( std::string( "dataPathSuffix" ), "" );


	g_eventSystem->RegisterMethodEvent( "print_bytecode_chunk", "Usage: print_bytecode_chunk entityName=<> chunkName=<>", eUsageLocation::DEV_CONSOLE, this, &Game::PrintBytecodeChunk );

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

	m_uiSystem->Shutdown();

	// Clean up member variables
	PTR_SAFE_DELETE( m_uiFontMaterial );
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_worldCamera );
	PTR_SAFE_DELETE( m_uiSystem );
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
				}
				break;

				case 1:
				{
					LoadAssets();
					InitializeUIElements();
					ChangeGameState( eGameState::ATTRACT );
					Update();
				}
				break;
			}
		}
		break;

		case eGameState::ATTRACT:
		case eGameState::PAUSED:
		{
			m_uiSystem->Update();
			UpdateFromKeyboard();
		}
		break;

		case eGameState::PLAYING:
		{
			m_uiSystem->Update();
			UpdateFromKeyboard();

			m_world->Update();
			g_physicsSystem2D->Update();

			if ( m_curInventoryDragItem != nullptr )
			{
				m_curInventoryDragItem->SetPosition( m_mouseUIPosition );
			}
		}
		break;

		case eGameState::GAME_OVER:
		case eGameState::VICTORY:
		{
			m_uiSystem->Update();
			UpdateFromKeyboard();

			m_world->Update();
			g_physicsSystem2D->Update();
		}
		break;
	}

	UpdateTimers();
	UpdateCameras();
	UpdateMousePositions();
	UpdateFramesPerSecond();

}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_gameState )
	{
		case eGameState::PLAYING:
		case eGameState::PAUSED:
		case eGameState::GAME_OVER:
		case eGameState::VICTORY:
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
			std::vector<Vertex_PCU> textVerts;

			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 550.f, 400.f ), 200.f, "Loading..." );
			g_renderer->BindMaterial( m_uiFontMaterial );
			g_renderer->BindDiffuseTexture( m_uiFont->GetTexture() );
			g_renderer->DrawVertexArray( textVerts );
		}
		break;

		case eGameState::ATTRACT:
		{
			std::vector<Vertex_PCU> textVerts;

			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 350.f, 550.f ), 200.f, m_gameName, Rgba8::RED );
			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 575.f, 300.f ), 75.f, "Press Any Key to Start" );
			g_renderer->BindMaterial( m_uiFontMaterial );
			g_renderer->BindDiffuseTexture( m_uiFont->GetTexture() );
			g_renderer->DrawVertexArray( textVerts );
		}
		break;

		case eGameState::PAUSED:
		{
			// TODO: Use UI system for main menus
			Rgba8 backgroundTint = Rgba8::BLACK;
			backgroundTint.a = 150;
			DrawAABB2( g_renderer, AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ), backgroundTint );

			std::vector<Vertex_PCU> textVerts;

			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 675.f, 500.f ), 200.f, "Paused" );
			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 450.f, 400.f ), 75.f, "Esc to Quit, Space to Resume" );
			g_renderer->BindMaterial( m_uiFontMaterial );
			g_renderer->BindDiffuseTexture( m_uiFont->GetTexture() );
			g_renderer->DrawVertexArray( textVerts );
		}
		break;

		case eGameState::GAME_OVER:
		{
			// TODO: Use UI system for main menus
			Rgba8 backgroundTint = Rgba8::BLACK;
			backgroundTint.a = 150;
			DrawAABB2( g_renderer, AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ), backgroundTint );

			std::vector<Vertex_PCU> textVerts;

			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 600.f, 500.f ), 200.f, "You Died" );
			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 450.f, 400.f ), 75.f, "Esc to Quit, Space to Restart" );
			g_renderer->BindMaterial( m_uiFontMaterial );
			g_renderer->BindDiffuseTexture( m_uiFont->GetTexture() );
			g_renderer->DrawVertexArray( textVerts );
		}
		break;

		case eGameState::VICTORY:
		{
			// TODO: Use UI system for main menus
			Rgba8 backgroundTint = Rgba8::BLACK;
			backgroundTint.a = 150;
			DrawAABB2( g_renderer, AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ), backgroundTint );

			std::vector<Vertex_PCU> textVerts;

			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 150.f, 500.f ), 125.f, "You Vanquished the Dragon!" );
			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 700.f, 400.f ), 75.f, Stringf( "Death Count: %i", m_numDeaths ) );
			m_uiFont->AppendVertsForText2D( textVerts, Vec2( 450.f, 250.f ), 75.f, "Esc to Quit, Space to Restart" );
			g_renderer->BindMaterial( m_uiFontMaterial );
			g_renderer->BindDiffuseTexture( m_uiFont->GetTexture() );
			g_renderer->DrawVertexArray( textVerts );
		}
		break;
	}

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

	if ( m_curInventoryDragItem != nullptr )
	{
		m_curInventoryDragItem->RenderInUI();
	}

	//RenderFPSCounter();

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

	LoadLevelThresholdElementsFromXml();
	LoadSounds();
	LoadUIElementsFromXml();

	LoadAndCompileZephyrScripts();
	LoadEntitiesFromXml();
	LoadWorldDefinitionFromXml();
	
	LoadTileMaterialsFromXml();
	LoadTilesFromXml();
	LoadMapsFromXml();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadSounds()
{
	g_devConsole->PrintString( "Loading Audio..." );

	std::string folderPath( "Data/Audio" );

	Strings audioFiles = GetFileNamesInFolder( folderPath, "*.*" );
	for ( int soundIdx = 0; soundIdx < (int)audioFiles.size(); ++soundIdx )
	{
		std::string soundName = GetFileNameWithoutExtension( audioFiles[soundIdx] );
		std::string& soundNameWithExtension = audioFiles[soundIdx];

		std::string soundFullPath( folderPath );
		soundFullPath += "/";
		soundFullPath += soundNameWithExtension;

		m_loadedSoundIds[soundName] = g_audioSystem->CreateOrGetSound( soundFullPath );
	}

	g_devConsole->PrintString( "Audio Files Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTileMaterialsFromXml()
{
	g_devConsole->PrintString( "Loading Tile Materials..." );

	const char* filePath = "Data/Gameplay/TileMaterialDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "Data/Gameplay/TileMaterialDefs.xml: Could not be opened." );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "TileMaterialDefinitions" ) )
	{
		g_devConsole->PrintError( "TileMaterialDefs.xml: Incorrect root node name, must be TileMaterialDefinitions" );
		return;
	}

	// Parse spritesheet
	std::string spriteSheetPath = ParseXmlAttribute( *root, "spriteSheet", "" );
	if ( spriteSheetPath == "" )
	{
		g_devConsole->PrintError( "TileMaterialDefs.xml: Missing a spriteSheet attribute" );
		return;
	}
	
	SpriteSheet* spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
	if ( spriteSheet == nullptr )
	{
		IntVec2 spriteSheetDimensions( -1, -1 );
		spriteSheetDimensions = ParseXmlAttribute( *root, "spriteSheetDimensions", spriteSheetDimensions );
		if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
		{
			g_devConsole->PrintError( "TileMaterialDefs.xml: Missing a spriteSheetDimensions attribute" );
			return;
		}

		Texture* texture = g_renderer->CreateOrGetTextureFromFile( ( "Data/" + spriteSheetPath ).c_str() );
		if ( texture == nullptr )
		{
			g_devConsole->PrintError( Stringf( "TileMaterialDefs.xml: Couldn't load texture '%s'", spriteSheetPath.c_str() ) );
			return;
		}
	
		spriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		TileMaterialDefinition* tileMatDef = new TileMaterialDefinition( *element, spriteSheet );

		if ( tileMatDef->IsValid() )
		{
			TileMaterialDefinition::s_definitions[tileMatDef->GetName()] = tileMatDef;
		}
		else
		{
			PTR_SAFE_DELETE( tileMatDef );
		}

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Tile Materials Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTilesFromXml()
{
	g_devConsole->PrintString( "Loading Tiles..." );

	const char* filePath = "Data/Gameplay/TileDefs.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "The tiles xml file '%s' could not be opened.", filePath ) );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "TileDefinitions" ) )
	{
		g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be TileDefinitions", filePath ) );
		return;
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		TileDefinition* tileDef = new TileDefinition( *element, m_defaultTileMaterialName );

		if ( tileDef->IsValid() )
		{
			TileDefinition::s_definitions[tileDef->GetName()] = tileDef;
		}
		else
		{
			PTR_SAFE_DELETE( tileDef );
		}

		element = element->NextSiblingElement();
	}

	g_devConsole->PrintString( "Tiles Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadMapsFromXml()
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

		MapData mapData( *root, GetFileNameWithoutExtension( mapName ), m_defaultTileName );

		if ( mapData.isValid )
		{
			m_world->AddNewMap( mapData );
		}
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadEntitiesFromXml()
{
	g_devConsole->PrintString( "Loading Entity Types..." );

	const char* filePath = "Data/Gameplay/EntityTypes.xml";

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

	// Parse default sprite sheet
	std::string spriteSheetPath = ParseXmlAttribute( *root, "spriteSheet", "" );
	if ( spriteSheetPath == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: Missing spriteSheet attribute" );
		return;
	}

	SpriteSheet* defaultSpriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
	if ( defaultSpriteSheet == nullptr )
	{
		IntVec2 spriteSheetDimensions = ParseXmlAttribute( *root, "spriteSheetDimensions", IntVec2( -1, -1 ) );
		if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
		{
			g_devConsole->PrintError( "EntityTypes.xml: Missing layout attribute" );
			return;
		}

		Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
		if ( texture == nullptr )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml: Couldn't load texture '%s'", spriteSheetPath.c_str() ) );
			return;
		}

		defaultSpriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		if ( !strcmp( element->Name(), "Actor" )
			 || !strcmp( element->Name(), "ActorSpawner" )
			 || !strcmp( element->Name(), "Entity" )
			 || !strcmp( element->Name(), "Item" )
			 || !strcmp( element->Name(), "Projectile" )
			 || !strcmp( element->Name(), "Portal" ) )
		{
			EntityDefinition* entityTypeDef = new EntityDefinition( *element, defaultSpriteSheet );
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

	std::string playerActorName = g_gameConfigBlackboard.GetValue( std::string( "playerActorName" ), "" );
	if ( playerActorName.empty() )
	{
		g_devConsole->PrintError( "GameConfig.xml doesn't define a playerActorName" );
		return;
	}

	EntityDefinition* playerDef = EntityDefinition::GetEntityDefinition( playerActorName );
	if ( playerDef == nullptr )
	{
		g_devConsole->PrintError( "GameConfig.xml's playerActorName was not loaded from EntityTypes.xml" );
		return;
	}

	m_player = new Actor( *playerDef, nullptr );
	m_player->SetAsPlayer();

	// Must be saved before initializing zephyr script
	m_world->SaveEntityByName( m_player );
	m_player->CreateZephyrScript( *playerDef );

	g_devConsole->PrintString( "Entity Types Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadWorldDefinitionFromXml()
{
	g_devConsole->PrintString( "Loading World Definition..." );

	std::string filePath = Stringf( "Data/Gameplay/WorldDef.xml" );

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath.c_str() );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "The world xml file '%s' could not be opened.", filePath.c_str() ) );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "WorldDefinition" ) )
	{
		g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be WorldDefinition", filePath.c_str() ) );
		return;
	}

	// Parse entities node
	XmlElement* entitiesElement = root->FirstChildElement( "Entities" );
	XmlElement* entityElement = entitiesElement->FirstChildElement();
	while ( entityElement )
	{
		if ( !strcmp( entityElement->Name(), "Actor" )
			 || !strcmp( entityElement->Name(), "Entity" )
			 || !strcmp( entityElement->Name(), "Projectile" )
			 || !strcmp( entityElement->Name(), "Portal" )
			 || !strcmp( entityElement->Name(), "Pickup" ) )
		{
			std::string entityTypeStr = ParseXmlAttribute( *entityElement, "type", "" );
			if ( entityTypeStr.empty() )
			{
				g_devConsole->PrintError( Stringf( "'%s': %s is missing a type attribute", filePath.c_str(), entityElement->Name() ) );
				return;
			}

			EntityDefinition* entityTypeDef = EntityDefinition::GetEntityDefinition( entityTypeStr );
			if ( entityTypeDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "'%s': Entity type '%s' was not defined in EntityTypes.xml", filePath.c_str(), entityTypeStr.c_str() ) );
				return;
			}

			m_world->AddEntityFromDefinition( *entityTypeDef );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "WorldDef.xml: Unsupported node '%s'", entityElement->Name() ) );
		}

		entityElement = entityElement->NextSiblingElement();
	}

	g_devConsole->PrintString( "World Definition Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAndCompileZephyrScripts()
{
	g_devConsole->PrintString( "Loading Zephyr Scripts..." );

	std::string folderPath( "Data/Scripts" + m_dataPathSuffix );

	Strings scriptFiles = GetFileNamesInFolder( folderPath, "*.zephyr" );
	for ( int scriptIdx = 0; scriptIdx < (int)scriptFiles.size(); ++scriptIdx )
	{
		std::string& scriptName = scriptFiles[scriptIdx];

		std::string scriptFullPath( folderPath );
		scriptFullPath += "/";
		scriptFullPath += scriptName;

		// Save compiled script into static map
		ZephyrScriptDefinition* scriptDef = ZephyrCompiler::CompileScriptFile( scriptFullPath );
		scriptDef->m_name = scriptName;

		ZephyrScriptDefinition::s_definitions[scriptFullPath] = scriptDef;
	}

	g_devConsole->PrintString( "Zephyr Scripts Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadUIElementsFromXml()
{
	g_devConsole->PrintString( "Loading UI Elements..." );

	const char* filePath = "Data/UI/UI.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "UI.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	m_uiSystem->LoadUIElementsFromXML( *root );

	g_devConsole->PrintString( "UI Elements Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadLevelThresholdElementsFromXml()
{
	g_devConsole->PrintString( "Loading Level thresholds..." );

	const char* filePath = "Data/Gameplay/LevelThresholds.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "LevelThresholds.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	LevelThresholdDefinition::LoadLevelThresholdsFromXML( *root );

	g_devConsole->PrintString( "Level thresholds Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::ReloadGame()
{
	if ( m_curMusicId != (SoundPlaybackID)-1 )
	{
		g_audioSystem->StopSound( m_curMusicId );
	}

	m_world->Reset();

	PTR_VECTOR_SAFE_DELETE( m_timerPool );

	g_gameConfigBlackboard.Clear();
	PopulateGameConfig();
	m_startingMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_startingMapName );

	m_player = nullptr;

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( EntityDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( TileMaterialDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( TileDefinition::s_definitions );
	PTR_VECTOR_SAFE_DELETE( SpriteSheet::s_definitions );

	m_uiSystem->Shutdown();
	m_uiSystem->Startup( g_window, g_renderer, m_uiFont, m_uiFontMaterial );
	m_curInventoryDragItem = nullptr;
	m_numDeaths = 0;

	g_physicsSystem2D->Reset();
	m_loadedSoundIds.clear();

	LoadLevelThresholdElementsFromXml();
	LoadSounds();
	LoadUIElementsFromXml();
	LoadAndCompileZephyrScripts();
	LoadEntitiesFromXml();
	LoadWorldDefinitionFromXml();
	LoadTileMaterialsFromXml();
	LoadTilesFromXml();
	LoadMapsFromXml();

	InitializeUIElements();
	
	UIPanel* hudPanel = (UIPanel*)m_uiSystem->GetUIElementByName( "HUD" );
	hudPanel->Show();
	hudPanel->Activate();

	m_isInventoryOpen = false;
	
	EventArgs args;
	g_eventSystem->FireEvent( "GameStarted", &args );
	g_devConsole->PrintString( "Data files reloaded", Rgba8::GREEN );

}


//-----------------------------------------------------------------------------------------------
void Game::ReloadScripts()
{
	m_world->UnloadAllEntityScripts();

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );

	LoadAndCompileZephyrScripts();

	for ( auto& entityDef : EntityDefinition::s_definitions )
	{
		if ( entityDef.second != nullptr )
		{
			entityDef.second->ReloadZephyrScriptDefinition();
		}
	}

	m_world->ReloadAllEntityScripts();

	g_devConsole->PrintString( "Scripts reloaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	switch ( m_gameState )
	{
		case eGameState::ATTRACT: UpdateFromKeyboardAttract(); break;
		case eGameState::PLAYING: UpdateFromKeyboardPlaying(); break;
		case eGameState::PAUSED: UpdateFromKeyboardPaused(); break;
		case eGameState::GAME_OVER: UpdateFromKeyboardGameOver(); break;
		case eGameState::VICTORY: UpdateFromKeyboardVictory(); break;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboardPlaying()
{
	if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ESC ) > 0 )
	{
		if ( m_curInventoryDragItem != nullptr )
		{
			m_world->GetCurrentMap()->TakeOwnershipOfEntity( m_curInventoryDragItem );
			m_curInventoryDragItem->SetPosition( m_player->GetPosition() );

			m_curInventoryDragItem = nullptr;
			m_sourceInventorySlotId = -1;
		}
		else if ( m_isInventoryOpen )
		{
			UIElement* inventory = g_game->GetUISystem()->GetUIElementByName( "inventory" );
			if ( inventory == nullptr )
			{
				g_devConsole->PrintError( "No inventory found!" );
			}
			else
			{
				inventory->Deactivate();
				inventory->Hide();
				m_isInventoryOpen = false;
			}
		}
		else
		{
			ChangeGameState( eGameState::PAUSED );
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'I' ) 
		|| g_inputSystem->WasKeyJustPressed( KEY_TAB ) )
	{
		UIElement* inventory = g_game->GetUISystem()->GetUIElementByName( "inventory" );
		if ( inventory == nullptr )
		{
			g_devConsole->PrintError( "No inventory found!" );
		}
		else
		{
			if ( m_isInventoryOpen )
			{
				inventory->Deactivate();
				inventory->Hide();
				m_isInventoryOpen = false;
			}
			else
			{
				inventory->Show();
				inventory->Activate();
				m_isInventoryOpen = true;
			}
		}
	}

	/*if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}*/

	/*if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
	{
		m_player->GainXP( 1 );
	}*/

	if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
	{
		ReloadGame();
		LoadStartingMap( m_startingMapName );
	}

	if ( m_cameraMode == eCameraMode::FREE_MOVEMENT )
	{
		if ( g_inputSystem->IsKeyPressed( 'W' ) )
		{
			m_focalPoint.y += m_cameraSpeed * (float)m_gameClock->GetLastDeltaSeconds();
		}

		if ( g_inputSystem->IsKeyPressed( 'A' ) )
		{
			m_focalPoint.x -= m_cameraSpeed * (float)m_gameClock->GetLastDeltaSeconds();
		}

		if ( g_inputSystem->IsKeyPressed( 'S' ) )
		{
			m_focalPoint.y -= m_cameraSpeed * (float)m_gameClock->GetLastDeltaSeconds();
		}

		if ( g_inputSystem->IsKeyPressed( 'D' ) )
		{
			m_focalPoint.x += m_cameraSpeed * (float)m_gameClock->GetLastDeltaSeconds();
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_SHIFT ) )
	{
		switch ( m_cameraMode )
		{
			case eCameraMode::LOCKED_TO_PLAYER: m_cameraMode = eCameraMode::FREE_MOVEMENT; return;
			case eCameraMode::FREE_MOVEMENT: m_cameraMode = eCameraMode::LOCKED_TO_PLAYER; return;
		}
	}

	if ( g_inputSystem->WasKeyJustReleased( MOUSE_LBUTTON ) )
	{
		if ( m_curInventoryDragItem != nullptr )
		{
			SwapDragItemWithTargetSlot( m_uiSystem->GetUIElementById( m_sourceInventorySlotId ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboardAttract()
{
	if ( g_inputSystem->WasAnyKeyJustPressed() )
	{
		ChangeGameState( eGameState::PLAYING );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboardPaused()
{
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		g_eventSystem->FireEvent( "Quit" );
	}
	else if ( g_inputSystem->WasAnyKeyJustPressed() )
	{
		ChangeGameState( eGameState::PLAYING );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboardGameOver()
{
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		g_eventSystem->FireEvent( "Quit" );
	}
	else if ( g_inputSystem->WasKeyJustPressed( KEY_SPACEBAR ) )
	{
		m_player->Respawn();
		ChangeGameState( eGameState::PLAYING );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboardVictory()
{
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		g_eventSystem->FireEvent( "Quit" );
	}
	else if ( g_inputSystem->WasKeyJustPressed( KEY_SPACEBAR ) )
	{
		g_inputSystem->ConsumeAnyKeyJustPressed();
		ReloadGame();
		ChangeGameState( eGameState::ATTRACT );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMap( const std::string& mapName )
{
	m_world->ChangeMap( mapName, m_player );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadStartingMap( const std::string& mapName )
{
	m_world->InitializeAllZephyrEntityVariables();

	m_world->ChangeMap( mapName, m_player );

	if ( m_player != nullptr )
	{
		m_player->FireSpawnEvent();
	}

	m_world->CallAllZephyrSpawnEvents( m_player );
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
void Game::UpdateTimers()
{
	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		GameTimer*& gameTimer = m_timerPool[timerIdx];
		if ( gameTimer == nullptr )
		{
			continue;
		}

		if ( gameTimer->timer.IsRunning()
			 && gameTimer->timer.HasElapsed() )
		{
			if ( !gameTimer->callbackName.empty() )
			{
				if ( gameTimer->targetId == -1 )
				{
					g_eventSystem->FireEvent( gameTimer->callbackName, gameTimer->callbackArgs );
				}
				else
				{
					Entity* targetEntity = GetEntityById( gameTimer->targetId );
					if ( targetEntity != nullptr )
					{
						targetEntity->FireScriptEvent( gameTimer->callbackName, gameTimer->callbackArgs );
					}
				}
			}

			delete m_timerPool[timerIdx];
			m_timerPool[timerIdx] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::PlaySoundByName( const std::string& soundName, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	volume = volume > m_maxMasterVolume ? m_maxMasterVolume : volume;

	auto iter = m_loadedSoundIds.find( soundName );
	if ( iter == m_loadedSoundIds.end() )
	{
		g_devConsole->PrintError( Stringf( "Cannot play unregistered sound, '%s", soundName.c_str() ) );
		return;
	}

	SoundID soundId = iter->second;

	g_audioSystem->PlaySound( soundId, isLooped, volume, balance, speed, isPaused );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMusic( const std::string& musicName, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	volume = volume > m_maxMasterVolume ? m_maxMasterVolume : volume;

	auto iter = m_loadedSoundIds.find( musicName );
	if ( iter == m_loadedSoundIds.end() )
	{
		g_devConsole->PrintError( Stringf( "Cannot play unregistered music, '%s", musicName.c_str() ) );
		return;
	}

	SoundID soundId = iter->second;
	if ( m_curMusicId != (SoundPlaybackID)-1 )
	{
		g_audioSystem->StopSound( m_curMusicId );
	}

	m_curMusicName = musicName;
	m_curMusicId = g_audioSystem->PlaySound( soundId, isLooped, volume, balance, speed, isPaused );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMusicVolume( float newVolume )
{
	newVolume = newVolume > m_maxMasterVolume ? m_maxMasterVolume : newVolume;

	if ( m_curMusicId != (SoundPlaybackID)-1 )
	{
		g_audioSystem->SetSoundPlaybackVolume( m_curMusicId, newVolume );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeUIElements()
{
	UIPanel* hudPanel = (UIPanel*)m_uiSystem->GetUIElementByName( "HUD" );
	hudPanel->Deactivate();
	hudPanel->Hide();

	UIButton* attack1Btn = (UIButton*)m_uiSystem->GetUIElementByName( "attack1" );
	attack1Btn->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnHUDHoverBegin );
	attack1Btn->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnHUDHoverEnd );
	attack1Btn->m_onClickEvent.SubscribeMethod( this, &Game::OnAttackClickEvent );

	EventArgs scriptArgs;
	scriptArgs.SetValue( "buttonName", attack1Btn->GetName() );
	scriptArgs.SetValue( "buttonPos", attack1Btn->GetPosition() );
	m_player->FireScriptEvent( "OnInitHotbarPosition", &scriptArgs );
	

	/*UIButton* attack2Btn = (UIButton*)m_uiSystem->GetUIElementByName( "attack2" );
	attack2Btn->m_onHoverBeginEvent.SubscribeMethod( this, &Game::OnHUDHoverBegin );
	attack2Btn->m_onHoverEndEvent.SubscribeMethod( this, &Game::OnHUDHoverEnd );
	attack2Btn->m_onClickEvent.SubscribeMethod( this, &Game::OnAttackClickEvent );*/

	UIUniformGrid* inventoryGrid = (UIUniformGrid*)m_uiSystem->GetUIElementByName( "inventoryGrid" );
	if ( inventoryGrid != nullptr )
	{
		for ( UIElement* gridElem : inventoryGrid->GetGridElements() )
		{
			UIButton* elemAsButton = (UIButton*)gridElem;
			elemAsButton->m_onClickEvent.SubscribeMethod( this, &Game::OnInventoryClickEvent );
			elemAsButton->m_onReleaseEvent.SubscribeMethod( this, &Game::OnInventoryReleaseEvent );

			EventArgs* userData = new EventArgs();
			userData->SetValue( "itemType", "any" );
			elemAsButton->SetUserData( userData );
		}
	}

	
	InitializeEquipmentUISlot( "weaponSlot", eItemType::WEAPON );
	InitializeEquipmentUISlot( "armorSlot",	 eItemType::ARMOR );
	InitializeEquipmentUISlot( "shieldSlot", eItemType::SHIELD );
	InitializeEquipmentUISlot( "ring1Slot",  eItemType::RING );
	InitializeEquipmentUISlot( "ring2Slot",  eItemType::RING );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeEquipmentUISlot( const std::string& uiElemName, eItemType itemType )
{
	UIButton* equipmentSlot = (UIButton*)m_uiSystem->GetUIElementByName( uiElemName );
	equipmentSlot->m_onClickEvent.SubscribeMethod( this, &Game::OnEquipmentClickEvent );
	equipmentSlot->m_onReleaseEvent.SubscribeMethod( this, &Game::OnEquipmentReleaseEvent );

	EventArgs* userData = new EventArgs();
	userData->SetValue( "itemType", GetItemTypeAsString( itemType ) );
	equipmentSlot->SetUserData( userData );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeFPSHistory()
{
	// Optimistically initialize fps history to 60 fps
	for ( int i = 0; i < FRAME_HISTORY_COUNT; ++i )
	{
		m_fpsHistory[i] = 60.f;
		m_fpsHistorySum += 60.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFramesPerSecond()
{
	float lastFPS = (float)m_gameClock->GetLastDeltaSeconds();

	float curFPS;
	if ( lastFPS < .0001f )
	{
		curFPS = 9999999.f;
	}
	else
	{
		curFPS = 1.f / lastFPS;
	}

	m_fpsHistorySum -= m_fpsHistory[m_fpsNextIdx];
	m_fpsHistory[m_fpsNextIdx] = curFPS;
	m_fpsHistorySum += curFPS;

	++m_fpsNextIdx;
	if ( m_fpsNextIdx >= FRAME_HISTORY_COUNT - 1 )
	{
		m_fpsNextIdx = 0;
	}
}


//-----------------------------------------------------------------------------------------------
float Game::GetAverageFPS() const
{
	constexpr float oneOverFrameCount = 1.f / (float)FRAME_HISTORY_COUNT;

	return m_fpsHistorySum * oneOverFrameCount;
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
void Game::PrintBytecodeChunk( EventArgs* args )
{
	std::string entityName = args->GetValue( "entityName", "" );
	std::string chunkName = args->GetValue( "chunkName", "" );

	if ( entityName.empty()
		 || chunkName.empty() )
	{
		return;
	}

	Entity* entity = GetEntityByName( entityName );
	if ( entity == nullptr )
	{
		return;
	}

	const ZephyrBytecodeChunk* chunk = entity->GetBytecodeChunkByName( chunkName );
	if ( chunk == nullptr )
	{
		return;
	}

	chunk->Disassemble();
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
void Game::WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	// No entity specified, just load the new map and set camera position and orientation
	if ( entityToWarp == nullptr )
	{
		if ( destMapName != "" )
		{
			m_world->ChangeMap( destMapName, m_player );
		}
		
		return;
	}

	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
float Game::GetLastDeltaSecondsf()
{
	return (float)m_gameClock->GetLastDeltaSeconds();
}


//-----------------------------------------------------------------------------------------------
Entity* Game::GetEntityById( EntityId id )
{
	return m_world->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
Entity* Game::GetEntityByName( const std::string& name )
{
	return m_world->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
Map* Game::GetMapByName( const std::string& name )
{
	return m_world->GetMapByName( name );
}


//-----------------------------------------------------------------------------------------------
Map* Game::GetCurrentMap()
{
	if ( m_world == nullptr )
	{
		return nullptr;
	}

	return m_world->GetCurrentMap();
}


//-----------------------------------------------------------------------------------------------
void Game::SaveEntityByName( Entity* entity )
{
	m_world->SaveEntityByName( entity );
}


//-----------------------------------------------------------------------------------------------
void Game::StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs )
{
	GameTimer* newTimer = new GameTimer( m_gameClock, targetId, onCompletedEventName, name, callbackArgs );

	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		if ( m_timerPool[timerIdx] == nullptr )
		{
			m_timerPool[timerIdx] = newTimer;
			newTimer->timer.Start( (double)durationSeconds );
			return;
		}
	}

	newTimer->timer.Start( (double)durationSeconds );
	m_timerPool.push_back( newTimer );
}


//-----------------------------------------------------------------------------------------------
void Game::StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs )
{
	Entity* target = m_world->GetEntityByName( targetName );

	if ( target == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Couldn't start a timer event with unknown target name '%s'", targetName.c_str() ) );
		return;
	}

	StartNewTimer( target->GetId(), name, durationSeconds, onCompletedEventName, callbackArgs );
}


//-----------------------------------------------------------------------------------------------
float Game::GetTimerRemaining( const std::string& timerName )
{
	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		GameTimer*& gameTimer = m_timerPool[timerIdx];
		if ( gameTimer == nullptr )
		{
			continue;
		}

		if ( gameTimer->name == timerName )
		{
			return (float)gameTimer->timer.GetSecondsRemaining();
		}
	}

	return 0.f;
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
			ChangeMusic( "MainMenuMusic" );

			UIPanel* hudPanel = (UIPanel*)m_uiSystem->GetUIElementByName( "HUD" );
			hudPanel->Deactivate();
			hudPanel->Hide();
		}
		break;

		case eGameState::PLAYING:
		{
			// Check which state we are changing from
			switch ( m_gameState )
			{
				case eGameState::PAUSED:
				{
					PlaySoundByName( "Unpause" );
					m_gameClock->Resume();
					//ChangeMusicVolume( m_maxMasterVolume );
				}
				break;

				case eGameState::ATTRACT:
				{
					//ChangeMusic( "OverworldMusic" );
					
					g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMapName.c_str() ) );
					LoadStartingMap( m_startingMapName );

					UIPanel* hudPanel = (UIPanel*)m_uiSystem->GetUIElementByName( "HUD" );
					hudPanel->Show();
					hudPanel->Activate();

					EventArgs args;
					g_eventSystem->FireEvent( "GameStarted", &args );

					SetWorldCameraPosition( Vec3( m_player->GetPosition(), 0.f ) );
				}
				break;

				case eGameState::GAME_OVER:
				{
					EventArgs args;
					g_eventSystem->FireEvent( "GameRestarted", &args );

					//ChangeMusicVolume( m_maxMasterVolume );
				}
			}
		}
		break;

		case eGameState::PAUSED:
		{
			//ChangeMusicVolume( m_maxMasterVolume * .5f );
			m_gameClock->Pause();

			PlaySoundByName( "Pause" );

		}
		break;

		case eGameState::GAME_OVER:
		{
			++m_numDeaths;
			//ChangeMusicVolume( m_maxMasterVolume * .5f );

			//PlaySoundByName( "Pause" );

		}
		break;


		case eGameState::VICTORY:
		{
			//m_curVictoryScreenSeconds = 0.f;

			ChangeMusic( "Victory" );

			UIPanel* hudPanel = (UIPanel*)m_uiSystem->GetUIElementByName( "HUD" );
			hudPanel->Deactivate();
			hudPanel->Hide();

			m_player->ActivateInvincibility();
		}
		break;
	}

	m_gameState = newGameState;
}


//-----------------------------------------------------------------------------------------------
void Game::OnHUDHoverBegin( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	button->ActivateHoverTint();
}


//-----------------------------------------------------------------------------------------------
void Game::OnHUDHoverEnd( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	button->ResetTint();
}


//-----------------------------------------------------------------------------------------------
void Game::OnAttackClickEvent( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	EventArgs scriptArgs;
	scriptArgs.SetValue( "buttonName", button->GetName() );
	scriptArgs.SetValue( "buttonPos", button->GetPosition() );
	m_player->FireScriptEvent( "OnClickInHotbar", &scriptArgs );
}


//-----------------------------------------------------------------------------------------------
void Game::OnInventoryClickEvent( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	m_curInventoryDragItem = RemoveInventoryItemFromSlot( button );
	if ( m_curInventoryDragItem != nullptr )
	{
		m_sourceInventorySlotId = button->GetId();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::OnInventoryReleaseEvent( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	if ( m_curInventoryDragItem == nullptr )
	{
		return;
	}

	SwapDragItemWithTargetSlot( button );
}


//-----------------------------------------------------------------------------------------------
void Game::OnEquipmentClickEvent( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	m_curInventoryDragItem = RemoveInventoryItemFromSlot( button );
	if ( m_curInventoryDragItem != nullptr )
	{
		m_sourceInventorySlotId = button->GetId();
	}

	m_player->RemoveEquipment( m_curInventoryDragItem );
}


//-----------------------------------------------------------------------------------------------
void Game::OnEquipmentReleaseEvent( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	if ( m_curInventoryDragItem == nullptr )
	{
		return;
	}

	EventArgs* userData = button->GetUserData();
	if ( userData != nullptr )
	{
		Item* oldEquipment = (Item*)userData->GetValue( "item", ( void* )nullptr );
		Item* newEquipment = m_curInventoryDragItem;

		if ( SwapDragItemWithTargetSlot( button ) )
		{
			m_player->RemoveEquipment( oldEquipment );
			m_player->AddEquipment( newEquipment );
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Game::SwapDragItemWithTargetSlot( UIElement* targetSlot )
{
	bool didPlaceInNewSlot = false;

	EventArgs* userData = targetSlot->GetUserData();
	if ( userData == nullptr )
	{
		return false;
	}

	eItemType slotItemType = GetItemTypeFromString( userData->GetValue( "itemType", "any" ) );
	UIElement* sourceSlot = m_uiSystem->GetUIElementById( m_sourceInventorySlotId );

	// This equipment can be placed here
	if ( slotItemType == eItemType::ANY
		|| slotItemType == m_curInventoryDragItem->GetItemType() )
	{
		// Check if this needs to swap places with existing item
		Item* oldItem = (Item*)userData->GetValue( "item", ( void* )nullptr );
		if ( oldItem != nullptr )
		{
			if ( sourceSlot != nullptr )
			{
				AddItemToInventorySlot( oldItem, sourceSlot );
			}
		}

		targetSlot->ClearLabels();
		AddItemToInventorySlot( m_curInventoryDragItem, targetSlot );
		didPlaceInNewSlot = true;
	}
	else
	{
		if ( sourceSlot != nullptr )
		{
			AddItemToInventorySlot( m_curInventoryDragItem, sourceSlot );
		}
	}

	m_curInventoryDragItem = nullptr;
	m_sourceInventorySlotId = -1;

	return didPlaceInNewSlot;
}


//-----------------------------------------------------------------------------------------------
void Game::AddItemToInventorySlot( Item* item, UIElement* targetSlot )
{
	EventArgs* userData = new EventArgs();
	userData->SetValue( "item", (void*)item );
	userData->SetValue( "itemType", targetSlot->GetUserData()->GetValue( "itemType", "any" ) );
	targetSlot->SetUserData( userData );

	UIAlignedPositionData posData;
	targetSlot->AddImage( posData, const_cast<SpriteDefinition*>( item->GetUISpriteDef() ) );
}


//-----------------------------------------------------------------------------------------------
Item* Game::RemoveInventoryItemFromSlot( UIElement* targetSlot )
{
	EventArgs* userData = targetSlot->GetUserData();
	if ( userData != nullptr )
	{
		Item* item = (Item*)userData->GetValue( "item", ( void* )nullptr );

		EventArgs* newUserData = new EventArgs();
		newUserData->SetValue( "item", (void*)nullptr );
		newUserData->SetValue( "itemType", userData->GetValue( "itemType", "any" ) );
		targetSlot->SetUserData( newUserData );

		targetSlot->ClearLabels();

		return item;
	}

	return nullptr;
}


