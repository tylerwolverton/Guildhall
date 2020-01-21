#pragma once
class App;
class InputSystem;
class RenderContext;
class Game;
struct Vec2;
struct Rgba8;


//-----------------------------------------------------------------------------------------------
// External variable declarations
//
extern App* g_theApp;
extern InputSystem* g_theInput;
extern RenderContext* g_theRenderer;


enum EntityType
{
	ENTITY_TYPE_INVALID = -1,

	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_TRIANGLE,
	ENTITY_TYPE_PLAYER_TRIANGLE,
	ENTITY_TYPE_BOX,

	NUM_ENTITIY_TYPES
};

//-----------------------------------------------------------------------------------------------
// Game Constants
//
constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect window area

constexpr float WINDOW_WIDTH = 200.f;
constexpr float WINDOW_HEIGHT = 100.f;

constexpr float WORLD_SIZE_X = 640.f;
constexpr float WORLD_SIZE_Y = 320.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float PLAYER_KEYBOARD_SPEED = 50.f;
constexpr float PLAYER_CONTROLLER_SPEED = 5.f;
constexpr float PLAYER_MAX_SPEED = 75.f;
constexpr float PLAYER_SHOOT_SPEED = 100.f;

constexpr float TRIANGLE_SPAWN_TIMER = 8.f;
constexpr float BOX_SPAWN_TIMER = 10.f;
constexpr float BOX_SPAWN_COUNT = 6.f;

constexpr float ENEMY_SPEED = 30.f;

constexpr int TILE_SIZE = 32;

constexpr float DEBUG_LINE_THICKNESS = 0.15f;

constexpr int NUM_DEBRIS_VERTS = 12;
constexpr float DEBRIS_PHYSICS_RADIUS = 1.f;
constexpr float DEBRIS_COSMETIC_RADIUS = 1.f;
constexpr float DEBRIS_LIFESPAN_SECONDS = 1.f;


constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;

constexpr float HIT_FLASH_SECONDS = .3f;

//-----------------------------------------------------------------------------------------------
// Debug Drawing
//
void DrawLine(const Vec2& start, const Vec2& end, const Rgba8& color, float thickness);
void DrawRing(const Vec2& center, float radius, const Rgba8& color, float thickness);