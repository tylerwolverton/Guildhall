#pragma once
#include <string>

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
enum class eVerbState : int
{
	NONE,

	PICKUP,
	OPEN,
	CLOSE,
	TALK_TO,
	GIVE_TO_SOURCE,
	GIVE_TO_DESTINATION,

	LAST_VAL = GIVE_TO_DESTINATION
};

std::string GetVerbStateAsString( eVerbState verbState );
eVerbState GetVerbStateFromString( const std::string& typeStr );
std::string GetDisplayNameForVerbState( eVerbState verbState );

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
constexpr float UI_DEBUG_LINE_THICKNESS = 2.f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;


//-----------------------------------------------------------------------------------------------
// EventNames
const std::string OnPickUpVerbEventName = "OnPickUpVerb";
const std::string OnOpenVerbEventName = "OnOpenVerb";
const std::string OnCloseVerbEventName = "OnCloseVerb";
const std::string OnTalkToVerbEventName = "OnTalkToVerb";
const std::string OnGiveToSourceVerbEventName = "OnGiveToSourceVerb";
const std::string OnGiveToDestinationVerbEventName = "OnGiveToDestinationVerbVerb";

std::string GetEventNameForVerbState( eVerbState verbState );
