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
#include "Game/GameEvents.hpp"
#include "Game/GameJobs.hpp"
#include "Game/MapData.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	Entity::s_nextEntityId = 0;
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	if ( m_isGameStarted )
	{
		return;
	}

	Transform::s_axisOrientation.m_axisYawPitchRollOrder = eAxisYawPitchRollOrder::ZYX;

	Transform::s_identityOrientation.PushTransform( Mat44::CreateZRotationDegrees( -90.f ) );
	Transform::s_identityOrientation.PushTransform( Mat44::CreateXRotationDegrees( 90.f ) );

	g_eventSystem->RegisterMethodEvent( "warp", "Usage: warp <map=string> <pos=float,float> <yaw=float>", eUsageLocation::DEV_CONSOLE, this, &Game::WarpMapCommand );
			
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();

	m_world = new World( m_gameClock );
		
	LoadAssets();
	
	m_curMapStr = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMapStr );
	g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMapStr.c_str() ) );
	m_world->ChangeMap( m_curMapStr );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
	m_isGameStarted = true;
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	Transform::s_identityOrientation = Mat44::IDENTITY;

	SpriteSheet::s_definitions.clear();
	TileDefinition::s_definitions.clear();
	EntityDefinition::s_definitions.clear();
	MapMaterialTypeDefinition::s_definitions.clear();
	MapRegionTypeDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_gameClock );
	PTR_SAFE_DELETE( m_rng );
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
	m_world->Update();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateWorldMesh()
{
	m_world->UpdateMesh();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{

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
void Game::WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	// No entity specified, just load the new map and set camera position and orientation
	if ( entityToWarp == nullptr )
	{
		if ( destMapName != "" )
		{
			m_world->ChangeMap( destMapName );
		}
		
		return;
	}

	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
Entity* Game::CreateEntityInCurrentMap( eEntityType entityType, const Vec2& position, float yawOrientationDegrees )
{
	return m_world->CreateEntityInCurrentMap( entityType, position, yawOrientationDegrees );
}


//-----------------------------------------------------------------------------------------------
std::vector<Entity*> Game::GetEntitiesInCurrentMap()
{
	return m_world->GetEntitiesInCurrentMap();
}


//-----------------------------------------------------------------------------------------------
void Game::DeleteAllEntities()
{
	m_world->DeleteAllEntities();
}


//-----------------------------------------------------------------------------------------------
void Game::MovePlayer( Entity* player, const Vec2& translationVec )
{
	if ( player == nullptr )
	{
		return;
	}

	player->Translate( translationVec );
}


//-----------------------------------------------------------------------------------------------
void Game::MoveEntity( EntityId entityId, const Vec2& translationVec )
{
	Entity* entity = m_world->GetEntityById( entityId );

	if ( entity == nullptr )
	{
		return;
	}

	entity->Translate( translationVec );
}


//-----------------------------------------------------------------------------------------------
void Game::RotateEntity( EntityId entityId, float yawRotationDegrees )
{
	Entity* entity = m_world->GetEntityById( entityId );

	if ( entity == nullptr )
	{
		return;
	}

	entity->RotateYawDegrees( yawRotationDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::SetEntityPosition( EntityId entityId, const Vec2& newPosition )
{
	Entity* entity = m_world->GetEntityById( entityId );

	if ( entity == nullptr )
	{
		return;
	}

	entity->SetPosition( newPosition );
}


//-----------------------------------------------------------------------------------------------
void Game::SetEntityOrientation( EntityId entityId, float yawOrientationDegrees )
{
	Entity* entity = m_world->GetEntityById( entityId );

	if ( entity == nullptr )
	{
		return;
	}
	
	entity->SetOrientationDegrees( yawOrientationDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::PossessEntity( Entity*& player, const Transform& cameraTransform )
{
	Entity* entity = m_world->GetClosestEntityInSector( cameraTransform.GetPosition().XY(), cameraTransform.GetYawDegrees(), 90.f, 2.f );
	if ( entity == nullptr )
	{
		return;
	}

	if ( GetDistance3D( cameraTransform.GetPosition(), Vec3( entity->GetPosition(), entity->GetHeight() * .5f ) ) < 2.f )
	{
		player = m_world->GetClosestEntityInSector( cameraTransform.GetPosition().XY(), cameraTransform.GetYawDegrees(), 90.f, 2.f );

		player->Possess();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UnpossessEntity( Entity*& player )
{
	if ( player != nullptr )
	{
		player->Unpossess();
	}
	
	player = nullptr;
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

	/*Vec2 newPos = args->GetValue( "pos", m_worldCamera->GetTransform().GetPosition().XY() );
	float newYawDegrees = args->GetValue( "yaw", m_worldCamera->GetTransform().GetYawDegrees() );

	SetCameraPositionAndYaw( newPos, newYawDegrees );*/
}
