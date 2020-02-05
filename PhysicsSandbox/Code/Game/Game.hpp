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
class Entity;
struct Rgba8;
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
	void RenderGameObjects() const;
	void RenderPolygonPoints() const;
	
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );
	void UpdateMouse();
	void UpdateGameObjects();
	void UpdateDraggedObject();
	void UpdatePotentialPolygon();

	void SpawnDisc( const Vec2& center, float radius );
	void SpawnPolygon( const std::vector<Vec2>& points );
	void SpawnPolygon( const Polygon2& polygon );

	GameObject* GetTopGameObjectAtMousePosition();
	int GetIndexOfTopGameObjectAtMousePosition();
	int GetIndexOfGameObject( GameObject* gameObjectToFind );

	void PerformGarbageCollection();

private:
	bool				m_isDebugRendering		= false;

	Camera*				m_worldCamera			= nullptr;
	Vec3				m_focalPoint			= Vec3::ZERO;
	float				m_zoomFactor			= 1.f;

	Physics2D*			m_physics2D				= nullptr;

	Vec2				m_mouseWorldPosition	= Vec2::ZERO;
	   
	bool				m_isMouseDragging		= false;
	Vec2				m_dragOffset			= Vec2::ZERO;
	GameObject*			m_dragTarget			= nullptr;

	eGameState			m_gameState				= eGameState::SANDBOX;

	std::vector<Vec2>	m_potentialPolygonPoints;
	bool				m_isPotentialPolygonConvex = false;

	std::vector<GameObject*> m_gameObjects;
	std::vector<int> m_garbageGameObjectIndexes;
};
