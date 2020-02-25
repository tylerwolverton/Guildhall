#pragma once
class App;
class InputSystem;
class AudioSystem;
class RenderContext;
class Game;
class BitmapFont;
struct Vec2;
struct Rgba8;


//-----------------------------------------------------------------------------------------------
// External variable declarations
//
extern App* g_app;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern RenderContext* g_renderer;
extern Game* g_game;
extern BitmapFont* g_testFont;


//-----------------------------------------------------------------------------------------------
// Game Constants
//
constexpr float CLIENT_ASPECT = 16.f / 9.f;

constexpr float WINDOW_WIDTH = 16.f;
constexpr float WINDOW_HEIGHT = 9.f;
constexpr float WINDOW_WIDTH_PIXELS = 1920.f;
constexpr float WINDOW_HEIGHT_PIXELS = 1080.f;

constexpr float TILE_SIZE = 1.f;

constexpr int SAFE_ZONE_SIZE = 5;

constexpr float PLAYER_PHYSICS_RADIUS = .25f;
constexpr float PLAYER_COSMETIC_RADIUS = .4f;
constexpr float PLAYER_SPEED = .25f;
constexpr float PLAYER_MAX_SPEED = 1.f;
constexpr float PLAYER_MAX_TURN_SPEED = 180.f;

constexpr float PHYSICS_FRICTION_FRACTION = .2f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;

constexpr float HIT_FLASH_SECONDS = .3f;


//-----------------------------------------------------------------------------------------------
enum eScreenEdgesBitField : unsigned int
{
	SCREEN_EDGE_NONE	= 0,
	SCREEN_EDGE_RIGHT	= ( 1 << 0 ),
	SCREEN_EDGE_LEFT	= ( 1 << 1 ),
	SCREEN_EDGE_TOP		= ( 1 << 2 ),
	SCREEN_EDGE_BOTTOM	= ( 1 << 3 )
};