#include "Game/GameCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Scripting/GameAPI.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;						// Owned by Main_Windows.cpp
Window* g_window = nullptr;					// Owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Owned by the App
InputSystem* g_inputSystem = nullptr;		// Owned by the App
AudioSystem* g_audioSystem = nullptr;		// Owned by the App
Physics2D* g_physicsSystem2D = nullptr;		// Owned by the App
Game* g_game = nullptr;						// Owned by the App
GameAPI* g_gameAPI = nullptr;


//-----------------------------------------------------------------------------------------------
void PopulateGameConfig()
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( "Data/GameConfig.xml" );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		return;
	}

	XmlElement* root = doc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *root );
}


//-----------------------------------------------------------------------------------------------
eCollisionLayer GetCollisionLayerFromString( const std::string& layerStr )
{
	if ( !_stricmp( layerStr.c_str(), "StaticEnvironment" ) ) { return eCollisionLayer::STATIC_ENVIRONMENT; }
	if ( !_stricmp( layerStr.c_str(), "Player" ) ) { return eCollisionLayer::PLAYER; }
	if ( !_stricmp( layerStr.c_str(), "PlayerProjectile" ) ) { return eCollisionLayer::PLAYER_PROJECTILE; }
	if ( !_stricmp( layerStr.c_str(), "NPC" ) ) { return eCollisionLayer::NPC; }
	if ( !_stricmp( layerStr.c_str(), "Enemy" ) ) { return eCollisionLayer::ENEMY; }
	if ( !_stricmp( layerStr.c_str(), "EnemyProjectile" ) ) { return eCollisionLayer::ENEMY_PROJECTILE; }
	if ( !_stricmp( layerStr.c_str(), "Portal" ) ) { return eCollisionLayer::PORTAL; }
	if ( !_stricmp( layerStr.c_str(), "Pickup" ) ) { return eCollisionLayer::PICKUP; }

	return eCollisionLayer::NONE;
}

