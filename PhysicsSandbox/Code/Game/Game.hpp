#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
class Entity;
struct Rgba8;
class RandomNumberGenerator;
class Camera;
class Physics2D;
class GameObject;


//-----------------------------------------------------------------------------------------------
enum MouseState
{
	MOUSE_STATE_POINT,
	MOUSE_STATE_OBB2,

	MOUSE_STATE_NUM_STATES
};


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;
	void EndFrame();
	void Shutdown();

	void RestartGame();
	
public:
	RandomNumberGenerator* m_rng = nullptr;

private:

	void RenderMouseShape() const;
	void RenderShapes() const;
	
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );
	void UpdateMouse();
	void UpdateGameObjects();
	void UpdateDraggedObject();

	void SpawnDisc( const Vec2& center, float radius );

	GameObject* GetTopGameObjectAtMousePosition();
	int GetIndexOfTopGameObjectAtMousePosition();

private:
	bool		m_isDebugRendering = false;

	Camera*		m_worldCamera = nullptr;
	Camera*		m_uiCamera = nullptr;

	Physics2D*  m_physics2D = nullptr;

	MouseState  m_mouseState = MOUSE_STATE_POINT;
	Vec2		m_mouseWorldPosition = Vec2::ZERO;
	OBB2		m_mouseOBB2;
	   
	std::vector<GameObject*> m_gameObjects;
	std::vector<int> m_garbageGameObjectIndexes;


	bool m_isMouseDragging = false;
	GameObject* m_dragTarget = nullptr;
};
