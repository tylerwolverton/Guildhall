#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Rgba8;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class Physics2D;
class GameObject;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	SANDBOX,
	CREATE_POLYGON,
};


//-----------------------------------------------------------------------------------------------
struct MouseMovementHistoryPoint
{
	Vec2 position;
	float deltaSeconds;
};


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void BeginFrame();
	void Update();
	void Render() const;
	void DebugRender() const;
	void EndFrame();
	void Shutdown();

	void RestartGame();
	
public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void RenderGameObjects() const;
	void RenderPolygonPoints() const;
	void RenderUI() const;
	
	void UpdateFromKeyboard();
	void UpdateCameras();
	void UpdateMouse();
	void UpdateMouseHistory( const Vec2& position );
	void UpdateGameObjects();
	void UpdateDraggedObject();
	void UpdatePotentialPolygon();
	void UpdateOffScreenGameObjects();

	void ResetGameObjectColors();
	void CheckCollisions();
	void UpdateBorderColor( GameObject* gameObject );
	void HandleIntersection( GameObject* gameObject, GameObject* otherGameObject );

	void SpawnDisc( const Vec2& center, float radius );
	void SpawnPolygon( const std::vector<Vec2>& points );
	void SpawnPolygon( const Polygon2& polygon );

	GameObject* GetTopGameObjectAtMousePosition();
	int GetIndexOfTopGameObjectAtMousePosition();
	int GetIndexOfGameObject( GameObject* gameObjectToFind );

	void PerformGarbageCollection();
	MouseMovementHistoryPoint GetCummulativeMouseHistory();

private:
	Clock*				m_gameClock						= nullptr;
	bool				m_isDebugRendering				= false;
														
	Camera*				m_worldCamera					= nullptr;
	Camera*				m_uiCamera						= nullptr;
	Vec3				m_focalPoint					= Vec3::ZERO;
	float				m_zoomFactor					= 1.f;
														
	Physics2D*			m_physics2D						= nullptr;
														
	Vec2				m_mouseWorldPosition			= Vec2::ZERO;
	Vec2				m_lastMouseWorldPosition		= Vec2::ZERO;
	MouseMovementHistoryPoint m_mouseHistoryPoints[5];
	   											 
	bool				m_isMouseDragging				= false;
	Vec2				m_dragOffset					= Vec2::ZERO;
	GameObject*			m_dragTarget					= nullptr;
														
	eGameState			m_gameState						= eGameState::SANDBOX;

	std::vector<Vec2>	m_potentialPolygonPoints;
	bool				m_isPotentialPolygonConvex		= false;

	std::vector<GameObject*> m_gameObjects;
	std::vector<int> m_garbageGameObjectIndexes;
};
