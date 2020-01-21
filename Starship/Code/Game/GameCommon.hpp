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
extern App* g_app;
extern InputSystem* g_inputSystem;
extern RenderContext* g_renderer;


//-----------------------------------------------------------------------------------------------
// Game Constants
//
constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect window area

constexpr float WINDOW_WIDTH = 200.f;
constexpr float WINDOW_HEIGHT = 100.f;

constexpr float SHIP_INITIAL_Y = WINDOW_HEIGHT / 2; // Start with ship in the center of the screen vertically

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 100;
constexpr int MAX_ASTEROID_HEALTH = 3;
constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_ASTEROID_DEBRIS_PIECES = 7;
constexpr int NUM_ASTEROID_VERTS = 48;
constexpr float MIN_ASTEROID_SPAWN_DIST_FROM_PLAYER = 15.f;

constexpr int MAX_BULLETS = 200;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_BULLET_DEBRIS_PIECES = 3;

constexpr int MAX_BEETLES = 100;
constexpr int MAX_BEETLE_HEALTH = 3;
constexpr float BEETLE_SPEED = 10.f;
constexpr float BEETLE_PHYSICS_RADIUS = 1.8f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_BEETLE_DEBRIS_PIECES = 12;

constexpr int MAX_WASPS = 100;
constexpr int MAX_WASP_HEALTH = 2;
constexpr float WASP_ACCELERATION = 100.f;
constexpr float WASP_MAX_SPEED = 30.f;
constexpr float WASP_PHYSICS_RADIUS = 1.8f;
constexpr float WASP_COSMETIC_RADIUS = 2.0f;
constexpr int NUM_WASP_DEBRIS_PIECES = 12;

constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_MAX_SPEED = 50.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr int NUM_PLAYER_SHIP_DEBRIS_PIECES = 15;
constexpr float STARBURST_COOLDOWN = 4.f;
constexpr int NUM_STARBURST_BULLETS = 30;

constexpr float DEBUG_LINE_THICKNESS = 0.15f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;

constexpr int MAX_DEBRIS_PIECES = 500;
constexpr float DEBRIS_PHYSICS_RADIUS = 0.1f;
constexpr float DEBRIS_COSMETIC_RADIUS = 0.5f;
constexpr int NUM_DEBRIS_VERTS = 18;
constexpr float DEBRIS_LIFESPAN_SECONDS = 2.0f;

constexpr int NUM_WAVES = 5;

constexpr float GAME_RESET_DELAY_SECONDS = 3.0f;

constexpr float STARSHIP_TITLE_LINE_WIDTH = .8f;

constexpr float HIT_FLASH_SECONDS = .3f;

//-----------------------------------------------------------------------------------------------
// Debug Drawing
//
void DrawLine(const Vec2& start, const Vec2& end, const Rgba8& color, float thickness);
void DrawRing(const Vec2& center, float radius, const Rgba8& color, float thickness);