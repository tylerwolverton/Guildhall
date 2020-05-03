#pragma once
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Rgba8;
struct Manifold2;
class Physics2D;
class Rigidbody2D;
class Collision2D;
class RenderContext;


//-----------------------------------------------------------------------------------------------
enum eCollider2DType
{
	COLLIDER2D_NONE = -1,
	COLLIDER2D_DISC,
	COLLIDER2D_POLYGON,

	NUM_COLLIDER_TYPES
};


//-----------------------------------------------------------------------------------------------
// Interface for all Collider objects used with our Physics system
class Collider2D
{
	friend class Physics2D;

public: // Interface 
	int GetId()																	{ return m_id; }
	// cache off the world shape representation of this object
	// taking into account the owning rigidbody (if no owner, local is world)
	virtual void UpdateWorldShape() = 0;
	void ChangeFriction( float deltaFriction );

	// queries 
	virtual const Vec2 GetClosestPoint( const Vec2& pos ) const = 0;
	virtual bool Contains( const Vec2& pos ) const = 0;
	bool Intersects( const Collider2D* other ) const;

	Manifold2 GetCollisionManifold( const Collider2D* other ) const;
	float GetBounceWith( const Collider2D* otherCollider ) const;
	float GetFrictionWith( const Collider2D* otherCollider ) const;

	virtual float CalculateMoment( float mass ) = 0;

	virtual Vec2 GetFarthestPointInDirection( const Vec2& direction ) const = 0;

	// TODO: Move this to a generic AABB2 method
	virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const = 0;
	virtual const AABB2 GetWorldBounds() const																				{ return m_worldBounds; };

	// debug helpers
	virtual void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const = 0;

protected:
	virtual ~Collider2D() {}; // private - make sure this is virtual so correct deconstructor gets called

public:
	eCollider2DType m_type		= COLLIDER2D_NONE;  // keep track of the type - will help with collision later
	Rigidbody2D* m_rigidbody	= nullptr;			// owning rigidbody, used for calculating world shape
	PhysicsMaterial m_material;

	Vec2	m_localPosition; // my local offset from my parent
	Vec2	m_worldPosition; // calculated from local position and owning rigidbody if present

	Delegate<Collision2D> m_onOverlapEnterDelegate;
	Delegate<Collision2D> m_onOverlapStayDelegate;
	Delegate<Collision2D> m_onOverlapLeaveDelegate;

protected:
	Physics2D* m_system			= nullptr;			
	int m_id = -1;

	AABB2 m_worldBounds;
};
