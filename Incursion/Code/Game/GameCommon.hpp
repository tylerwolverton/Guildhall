#pragma once
class App;
class InputSystem;
class AudioSystem;
class RenderContext;
class Game;
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

constexpr int   PLAYER_MAX_LIVES = 4;
constexpr int	PLAYER_MAX_HEALTH = 15;
constexpr float	PLAYER_START_X = 2.f;
constexpr float	PLAYER_START_Y = 2.f;
constexpr float PLAYER_RESPAWN_SECONDS = 4.f;
constexpr float PLAYER_PHYSICS_RADIUS = .31f;
constexpr float PLAYER_COSMETIC_RADIUS = .42f;
constexpr float PLAYER_GUN_COSMETIC_RADIUS = PLAYER_COSMETIC_RADIUS - .05f;
constexpr float PLAYER_SPEED = .25f;
constexpr float PLAYER_MAX_SPEED = 1.f;
constexpr float PLAYER_MAX_TURN_SPEED = 180.f;
constexpr float PLAYER_GUN_TURN_SPEED = 270.f;

constexpr int   TANK_MAX_HEALTH = 2;
constexpr float TANK_PHYSICS_RADIUS = .31f;
constexpr float TANK_COSMETIC_RADIUS = .4f;
constexpr float TANK_SPEED = .25f;
constexpr float TANK_MAX_SPEED = .9f;
constexpr float TANK_MAX_TURN_SPEED = 180.f;
constexpr float TANK_WANDER_DIRECTION_CHANGE_COOLDOWN_SECONDS = 5.f;
constexpr float TANK_MIDDLE_WHISKER_LENGTH = .35f;
constexpr float TANK_SIDE_WHISKER_LENGTH = .5f;
constexpr float TANK_SIDE_WHISKER_ANGLE_DEGREES = 30.f;
constexpr float TANK_WHISKER_TURN_DEGREES = 10.f;
constexpr float TANK_WHISKER_SPEED_FACTOR = .25f;
constexpr float TANK_MAX_CHASE_RANGE = 10.f;
constexpr float TANK_SHOT_ANGLE_RANGE_DEGREES = 5.f;
constexpr float TANK_CHASE_ANGLE_RANGE_DEGREES = 45.f;
constexpr float TANK_SHOT_COOLDOWN = 1.7f;

constexpr int   TURRET_MAX_HEALTH = 3;
constexpr float TURRET_PHYSICS_RADIUS = .37f;
constexpr float TURRET_COSMETIC_RADIUS = .42f;
constexpr float TURRET_SEARCH_MAX_TURN_SPEED = 30.f;
constexpr float TURRET_SEARCH_SECTOR_APERTURE = 90.f;
constexpr float TURRET_MAX_ATTACK_RANGE = 15.f;
constexpr float TURRET_SHOT_ANGLE_RANGE_DEGREES = 5.f;
constexpr float TURRET_SHOT_COOLDOWN = 1.3f;

constexpr float BOULDER_PHYSICS_RADIUS = .4f;
constexpr float BOULDER_COSMETIC_RADIUS = .45f;

constexpr float BULLET_SPEED = 5.f;
constexpr float BULLET_PHYSICS_RADIUS = .1f;
constexpr float BULLET_COSMETIC_RADIUS = .1f;
constexpr float BULLET_SPRITE_SCALE = .1f;

constexpr float PHYSICS_FRICTION_FRACTION = .2f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;

constexpr float HIT_FLASH_SECONDS = .3f;

constexpr float END_GAME_TIMER_SECONDS = 1.f;
constexpr float DEATH_OVERLAY_TIMER_SECONDS = 2.f;
constexpr float DEATH_OVERLAY_DISMISSAL_TIMER_SECONDS = .5f;
constexpr float VICTORY_OVERLAY_TIMER_SECONDS = .5f;

//-----------------------------------------------------------------------------------------------
// Map Generation
// Map 1
constexpr int MAP1_WIDTH = 20;
constexpr int MAP1_HEIGHT = 30;
constexpr int MAP1_MUD_WORM_COUNT = 15;
constexpr int MAP1_MUD_WORM_LENGTH = 7;
constexpr int MAP1_STONE1_WORM_COUNT = 10;
constexpr int MAP1_STONE1_WORM_LENGTH = 7;
constexpr int MAP1_STONE2_WORM_COUNT = 27;
constexpr int MAP1_STONE2_WORM_LENGTH = 3;
constexpr int MAP1_MIN_TANK_COUNT = 4;
constexpr int MAP1_MAX_TANK_COUNT = 7;
constexpr int MAP1_MIN_TURRET_COUNT = 3;
constexpr int MAP1_MAX_TURRET_COUNT = 7;
constexpr int MAP1_MIN_BOULDER_COUNT = 2;
constexpr int MAP1_MAX_BOULDER_COUNT = 6;

//Map 2
constexpr int MAP2_WIDTH = 16;
constexpr int MAP2_HEIGHT = 50;
constexpr int MAP2_WET_SAND_WORM_COUNT = 7;
constexpr int MAP2_WET_SAND_WORM_LENGTH = 7; 
constexpr int MAP2_CONCRETE_WORM_COUNT = 30;
constexpr int MAP2_CONCRETE_WORM_LENGTH = 2;
constexpr int MAP2_WATER1_WORM_COUNT = 20;
constexpr int MAP2_WATER1_WORM_LENGTH = 12;
constexpr int MAP2_WATER2_WORM_COUNT = 3;
constexpr int MAP2_WATER2_WORM_LENGTH = 15;
constexpr int MAP2_MIN_TANK_COUNT = 20;
constexpr int MAP2_MAX_TANK_COUNT = 27;
constexpr int MAP2_MIN_TURRET_COUNT = 2;
constexpr int MAP2_MAX_TURRET_COUNT = 5;
constexpr int MAP2_MIN_BOULDER_COUNT = 100;
constexpr int MAP2_MAX_BOULDER_COUNT = 125;

//Map 3
constexpr int MAP3_WIDTH = 40;
constexpr int MAP3_HEIGHT = 30;
constexpr int MAP3_MUD_WORM_COUNT = 15;
constexpr int MAP3_MUD_WORM_LENGTH = 7;
constexpr int MAP3_WOOD1_WORM_COUNT = 155;
constexpr int MAP3_WOOD1_WORM_LENGTH = 4;
constexpr int MAP3_WOOD2_WORM_COUNT = 3;
constexpr int MAP3_WOOD2_WORM_LENGTH = 28;
constexpr int MAP3_MIN_TANK_COUNT = 20;
constexpr int MAP3_MAX_TANK_COUNT = 25;
constexpr int MAP3_MIN_TURRET_COUNT = 15;
constexpr int MAP3_MAX_TURRET_COUNT = 25;
constexpr int MAP3_MIN_BOULDER_COUNT = 10;
constexpr int MAP3_MAX_BOULDER_COUNT = 15;
