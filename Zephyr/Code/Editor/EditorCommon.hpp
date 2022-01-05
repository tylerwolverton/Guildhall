#pragma once
#include <string>

class Window;
class App;
class InputSystem;
class RenderContext;
class Editor;


//-----------------------------------------------------------------------------------------------
// External variable declarations
//
extern Window* g_window;
extern App* g_app;
extern InputSystem* g_inputSystem;
extern RenderContext* g_renderer;
extern Editor* g_editor;


//-----------------------------------------------------------------------------------------------
// Global Functions
//
void PopulateEditorConfig();


//-----------------------------------------------------------------------------------------------
// Game Constants
//
constexpr float CLIENT_ASPECT = 16.f / 9.f;
constexpr int FRAME_HISTORY_COUNT = 200;

constexpr float SQRT_2_OVER_2 = 0.70710678f;

constexpr float WINDOW_WIDTH = 16.f;
constexpr float WINDOW_HEIGHT = 9.f;
constexpr float WINDOW_WIDTH_PIXELS = 1920.f;
constexpr float WINDOW_HEIGHT_PIXELS = 1080.f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;
