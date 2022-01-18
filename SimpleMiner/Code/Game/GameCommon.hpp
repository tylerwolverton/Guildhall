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
// Game Constants
//
constexpr float CLIENT_ASPECT = 16.f / 9.f;

constexpr float WINDOW_WIDTH = 16.f;
constexpr float WINDOW_HEIGHT = 9.f;
constexpr float WINDOW_WIDTH_PIXELS = 1920.f;
constexpr float WINDOW_HEIGHT_PIXELS = 1080.f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float BLOCK_SIZE = 1.f;

constexpr int CHUNK_WIDTH = 16;
constexpr int CHUNK_LENGTH = 16;
constexpr int CHUNK_HEIGHT = 128;
constexpr int NUM_BLOCKS_IN_CHUNK = CHUNK_WIDTH * CHUNK_LENGTH * CHUNK_HEIGHT;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;
