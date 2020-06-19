#pragma once
class Window;
class App;
class InputSystem;
class AudioSystem;
class RenderContext;
class Game;
class SpriteSheet;
struct Vec2;
struct Rgba8;


//-----------------------------------------------------------------------------------------------
// External variable declarations
//
extern Window* g_window;
extern App* g_app;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern RenderContext* g_renderer;
extern Game* g_game;


//-----------------------------------------------------------------------------------------------
// Global Tile Sheets
//
extern SpriteSheet* g_tileSpriteSheet;
extern SpriteSheet* g_characterSpriteSheet;
extern SpriteSheet* g_portraitSpriteSheet;


//-----------------------------------------------------------------------------------------------
enum class eVerbState
{
	NONE,
	PICKUP,
	OPEN,
	CLOSE,
	TALK_TO,
	GIVE_TO_SOURCE,
	GIVE_TO_DESTINATION,
};


////-----------------------------------------------------------------------------------------------
//std::string GetVerbStateAsString( eVerbState verbState )
//{
//	switch ( verbState )
//	{
//		case eVerbState::NONE: return "None";
//		case eVerbState::PICKUP: return "PickUp";
//		case eVerbState::OPEN: return "Open";
//		case eVerbState::CLOSE: return "Close";
//		case eVerbState::TALK_TO: return "Talk to";
//		case eVerbState::GIVE_TO_SOURCE: return "Give to source";
//		case eVerbState::GIVE_TO_DESTINATION: return "Give to destination";
//		default: return "Unknown State";
//	}
//}


//-----------------------------------------------------------------------------------------------
// Game Constants
//
constexpr float CLIENT_ASPECT = 16.f / 9.f;

constexpr float WINDOW_WIDTH = 16.f;
constexpr float WINDOW_HEIGHT = 9.f;
constexpr float WINDOW_WIDTH_PIXELS = 1920.f;
constexpr float WINDOW_HEIGHT_PIXELS = 1080.f;

constexpr float TILE_SIZE = 1.f;

constexpr float PLAYER_PHYSICS_RADIUS = .25f;
constexpr float PLAYER_COSMETIC_RADIUS = .4f;
constexpr float PLAYER_SPEED = .25f;
constexpr float PLAYER_MAX_SPEED = 1.f;

constexpr float ACTOR_WANDER_DIRECTION_CHANGE_COOLDOWN_SECONDS = 3.f;

constexpr float PHYSICS_FRICTION_FRACTION = .2f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;
