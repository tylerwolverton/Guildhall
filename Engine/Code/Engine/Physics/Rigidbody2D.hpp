#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct Rgba8;
class Physics2D;
class Collider2D;
class RenderContext;


//-----------------------------------------------------------------------------------------------
enum eSimulationMode : unsigned int
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

	Collider2D* GetCollider()														{ return m_collider; }
	void TakeCollider( Collider2D* collider ); // takes ownership of a collider (destroying my current one if present)

	uint GetLayer() const															{ return m_layer; }
	void SetLayer( uint layer )														{ m_layer = layer; }

	Vec2 GetVelocity()																{ return m_velocity; }
	void SetVelocity( const Vec2& velocity );
	
	Vec2 GetImpaceVelocityAtPoint( const Vec2& point );
	
	Vec2 GetVerletVelocity()														{ return m_verletVelocity; }

	Vec2 GetPosition()																{ return m_worldPosition; }
	Vec2 GetCenterOfMass() const;
	void SetPosition( const Vec2& position );
	void Translate2D( const Vec2& translation );

	void SetRotationDegrees( float newRotationDegrees );
	void RotateDegrees( float deltaDegrees );
	void ChangeAngularVelocity( float deltaRadians );
	void SetAngularVelocity( float newAngularVelocity );

	float GetMass() const															{ return m_mass; }
	void ChangeMass( float deltaMass );
	float GetInverseMass() const													{ return m_inverseMass; }

	float GetDrag() const															{ return m_drag; }
	void SetDrag( float drag )														{ m_drag = drag; }
	void ChangeDrag( float deltaDrag );

	void AddForce( const Vec2& force );
	void ApplyImpulseAt( const Vec2& impulse, const Vec2& worldPosition );
	void ApplyDragForce();

	void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const;

	bool IsEnabled() const															{ return m_isEnabled; }
	void Enable()																	{ m_isEnabled = true; }
	void Disable()																	{ m_isEnabled = false; }

	eSimulationMode GetSimulationMode()	const										{ return m_simulationMode; }
	void SetSimulationMode( eSimulationMode mode )									{ m_simulationMode = mode; }

	float GetAngularVelocity() const												{ return m_angularVelocity; }
	float GetOrientationDegrees() const;
	float GetOrientationRadians() const												{ return m_orientationRadians; }
	float GetMomentOfInertia() const												{ return m_moment; }

public:
	NamedProperties m_userProperties;

private:
	Physics2D* m_system = nullptr;			// which scene created/owns this object
	Vec2 m_worldPosition = Vec2::ZERO;		// where in the world is this rigidbody
	Collider2D* m_collider = nullptr;

	Vec2 m_forces = Vec2::ZERO;
	Vec2 m_velocity = Vec2::ZERO;
	Vec2 m_verletVelocity = Vec2::ZERO;

	float m_mass = 1.f;
	float m_inverseMass = 1.f;
	float m_drag = 0.f;

	float m_orientationRadians = 0.f;
	float m_angularVelocity = 0.f;
	float m_frameTorque = 0.f;
	float m_moment = 0.f;
	float m_inverseMoment = 1.f;

	bool m_isEnabled = true;
	eSimulationMode m_simulationMode = SIMULATION_MODE_DYNAMIC;

	uint m_layer = 0;

private:
	~Rigidbody2D(); // destroys the collider
};
