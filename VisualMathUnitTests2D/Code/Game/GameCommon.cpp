#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void PopulateGameConfig()
{

}

//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;						// Owned by Main_Windows.cpp
Window* g_window = nullptr;					// Owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Owned by the App
InputSystem* g_inputSystem = nullptr;		// Owned by the App
AudioSystem* g_audioSystem = nullptr;		// Owned by the App
Game* g_game = nullptr;						// Owned by the App

float g_windowWidth =		 WINDOW_WIDTH;
float g_windowHeight =		 WINDOW_HEIGHT;
float g_windowWidthPixels =  WINDOW_WIDTH_PIXELS;
float g_windowHeightPixels = WINDOW_HEIGHT_PIXELS;

float g_debugLineThickness = DEBUG_LINE_THICKNESS;