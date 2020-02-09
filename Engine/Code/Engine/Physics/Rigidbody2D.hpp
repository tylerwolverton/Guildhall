#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct Rgba8;
class Physics2D;
class Collider2D;
class RenderContext;


//-----------------------------------------------------------------------------------------------
enum eSimulationMode
{
	SIMULATION_MODE_STATIC,
	SIMULATION_MODE_KINEMATIC,
	SIMULATION_MODE_DYNAMIC
};


//-----------------------------------------------------------------------------------------------
class Rigidbody2D
{
	friend class Physics2D;

public:
	Rigidbody2D( float mass );

	void Update( float deltaSeconds );

	void Destroy(); // helper for destroying myself (uses owner to destroy self)

	void TakeCollider( Collider2D* collider ); // takes ownership of a collider (destroying my current one if present)

	Vec2 GetVelocity()																{ return m_velocity; }
	void SetVelocity( const Vec2& velocity );

	Vec2 GetPosition()																{ return m_worldPosition; }
	void SetPosition( const Vec2& position);

	void AddForce( const Vec2& force );

	void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const;

	bool IsEnabled() const															{ return m_isEnabled; }
	void Enable()																	{ m_isEnabled = true; }
	void Disable()																	{ m_isEnabled = false; }

	eSimulationMode GetSimulationMode()												{ return m_simulationMode; }
	void SetSimulationMode( eSimulationMode mode )									{ m_simulationMode = mode; }

public:
	Collider2D* m_collider = nullptr;

private:
	Physics2D* m_system = nullptr;			// which scene created/owns this object
	Vec2 m_worldPosition = Vec2::ZERO;		// where in the world is this rigidbody

	Vec2 m_forces = Vec2::ZERO;
	Vec2 m_velocity = Vec2::ZERO;
	float m_mass = 1.f;
	float m_inverseMass = 1.f;

	bool m_isEnabled = true;
	eSimulationMode m_simulationMode = SIMULATION_MODE_DYNAMIC;

private:
	~Rigidbody2D(); // destroys the collider
};
