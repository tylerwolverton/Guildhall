#pragma once
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct AABB2;
class Rigidbody2D;
class Collider2D;
enum eSimulationMode : unsigned int;

//-----------------------------------------------------------------------------------------------
class GameObject
{
public:
	~GameObject(); 
	
	void DebugRender();
	
	Rgba8 GetBorderColor()													{ return m_borderColor; }
	void SetBorderColor( const Rgba8& newColor )							{ m_borderColor = newColor; }
	Rgba8 GetFillColor()													{ return m_fillColor; }
	void SetFillColor( const Rgba8& newColor )								{ m_fillColor = newColor; }

	void SetRigidbody( Rigidbody2D* rigidbody );
	void SetCollider( Collider2D* collider );

	Vec2 GetVelocity() const;
	void SetVelocity( const Vec2& velocity );

	Vec2 GetVerletVelocity() const;

	Vec2 GetPosition();
	void SetPosition( const Vec2& position );
	
	bool IsPhysicsEnabled() const;
	void EnablePhysics();
	void DisablePhysics();

	eSimulationMode GetSimulationMode();
	void SetSimulationMode( eSimulationMode mode );

	uint GetLayer() const;
	void SetLayer( uint layer );

	bool Contains( const Vec2& point ) const;
	bool Intersects( const GameObject* otherGameObject ) const;

	// TODO: Move this to a generic AABB2 method
	unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const;
	const AABB2 GetBoundingBox() const;

	void RotateDegrees( float deltaDegrees );
	void ChangeBounciness( float deltaBounciness );
	void ChangeFriction( float deltaFriction );
	void ChangeMass( float deltaMass );
	void ChangeDrag( float deltaDrag );
	void ChangeAngularVelocity( float deltaAnglularVelocity );
	void SetRotationDegrees( float newDegrees );
	void SetAngularVelocity( float newAngularVelocity );

	float GetBounciness() const;
	float GetFriction() const;
	float GetMass() const;
	float GetMomentOfInertia() const;
	float GetDrag() const;
	float GetRotationDegrees() const;
	float GetAngularVelocity() const;

private:
	Rigidbody2D* m_rigidbody = nullptr;
	Rgba8 m_borderColor = Rgba8::MAGENTA;
	Rgba8 m_fillColor = Rgba8::MAGENTA;
};
