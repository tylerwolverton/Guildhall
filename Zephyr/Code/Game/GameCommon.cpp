#include "Game/GameCommon.hpp"
#include "Game/Scripting/GameAPI.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;						// Owned by Main_Windows.cpp
Window* g_window = nullptr;					// Owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Owned by the App
InputSystem* g_inputSystem = nullptr;		// Owned by the App
AudioSystem* g_audioSystem = nullptr;		// Owned by the App
Physics2D* g_physicsSystem2D = nullptr;		// Owned by the App
Game* g_game = nullptr;						// Owned by the App
GameAPI* g_gameAPI = nullptr;
