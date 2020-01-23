#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;
struct Rgba8;
class RandomNumberGenerator;
class Camera;


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
	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;
	void Shutdown();

	void RestartGame();
	
public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void RandomizeShapes();
	void RandomizeLine();
	void RandomizeDisc();
	void RandomizeAABB2();
	void RandomizeOBB2();
	void RandomizeCapsule2();

	void RenderMouseShape() const;
	void RenderShapes() const;
	void RenderLine() const;
	void RenderDisc() const;
	void RenderAABB2() const;
	void RenderOBB2() const;
	void RenderCapsule2() const;
	void RenderPolygon2() const;

	void RenderNearestPoints() const;

	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );
	void UpdateMousePosition();
	void UpdateNearestPoints();
	void UpdateShapeColors();

private:
	bool		m_isDebugRendering = false;

	Camera*		m_worldCamera = nullptr;
	Camera*		m_uiCamera = nullptr;

	MouseState  m_mouseState = MOUSE_STATE_POINT;
	Vec2		m_mouseWorldPosition = Vec2::ZERO;
	OBB2		m_mouseOBB2;

	Vec2		m_lineSegmentStart = Vec2::ZERO;
	Vec2		m_lineSegmentVector = Vec2::ZERO;

	Vec2		m_discCenter = Vec2::ZERO;
	float		m_discRadius = 0.f;

	AABB2		m_aabb2 = AABB2::ONE_BY_ONE;
	OBB2		m_obb2;
	Capsule2	m_capsule2;

	Vec2		m_nearestPointOnLine = Vec2::ZERO;
	Vec2		m_nearestPointOnDisc = Vec2::ZERO;
	Vec2		m_nearestPointOnAABB2 = Vec2::ZERO;
	Vec2		m_nearestPointOnOBB2 = Vec2::ZERO;
	Vec2		m_nearestPointOnCapsule2 = Vec2::ZERO;

	Rgba8		m_lineColor = Rgba8::CYAN;
	Rgba8		m_discColor = Rgba8::GREEN;
	Rgba8		m_AABB2Color = Rgba8::MAGENTA;
	Rgba8		m_OBB2Color = Rgba8::ORANGE;
	Rgba8		m_capsule2Color = Rgba8::RED;
};
