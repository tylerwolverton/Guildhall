#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Manifold2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


typedef bool ( *CollisionCheckCallback )( const Collider2D*, const Collider2D* );
typedef Manifold2( *CollisionManifoldGenerationCallback )( const Collider2D*, const Collider2D* );


//-----------------------------------------------------------------------------------------------
static bool DiscVDiscCollisionCheck( const Collider2D* collider1, const Collider2D* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider2D* discCollider1 = (const DiscCollider2D*)collider1;
	const DiscCollider2D* discCollider2 = (const DiscCollider2D*)collider2;

	return DoDiscsOverlap( discCollider1->m_worldPosition, 
						   discCollider1->m_radius, 
						   discCollider2->m_worldPosition,
						   discCollider2->m_radius );
}


//-----------------------------------------------------------------------------------------------
static bool DiscVPolygonCollisionCheck( const Collider2D* collider1, const Collider2D* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider2D* discCollider = (const DiscCollider2D*)collider1;
	const PolygonCollider2D* polygonCollider = (const PolygonCollider2D*)collider2;

	Vec2 nearestPoint = polygonCollider->GetClosestPoint( discCollider->m_worldPosition );

	return IsPointInsideDisc( nearestPoint, discCollider->m_worldPosition, discCollider->m_radius );
}


//-----------------------------------------------------------------------------------------------
static bool PolygonVPolygonCollisionCheck( const Collider2D* collider1, const Collider2D* collider2 )
{
	UNUSED( collider1 );
	UNUSED( collider2 );
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	/*const PolygonCollider2D* polygonCollider1 = (const PolygonCollider2D*)collider1;
	const PolygonCollider2D* polygonCollider2 = (const PolygonCollider2D*)collider2;*/

	return false;
}


//-----------------------------------------------------------------------------------------------
// a "matrix" lookup is just a 2D array
static CollisionCheckCallback g_CollisionChecks[NUM_COLLIDER_TYPES * NUM_COLLIDER_TYPES] = {
	/*             disc,                         polygon, */
	/*    disc */  DiscVDiscCollisionCheck,      nullptr,
	/* polygon */  DiscVPolygonCollisionCheck,   PolygonVPolygonCollisionCheck
};


//-----------------------------------------------------------------------------------------------
void Collider2D::ChangeFriction( float deltaFriction )
{
	m_material.m_friction += deltaFriction;

	m_material.m_friction = ClampZeroToOne( m_material.m_friction );
}


//-----------------------------------------------------------------------------------------------
bool Collider2D::Intersects( const Collider2D* other ) const
{
	if ( other == nullptr
		 || !m_rigidbody->IsEnabled()
		 || !other->m_rigidbody->IsEnabled()
		 || !DoAABBsOverlap2D( GetWorldBounds(), other->GetWorldBounds() ) )
	{
		return false;
	}

	eCollider2DType myType = m_type;
	eCollider2DType otherType = other->m_type;

	if ( myType <= otherType ) 
	{
		int idx = otherType * NUM_COLLIDER_TYPES + myType;
		CollisionCheckCallback callback = g_CollisionChecks[idx];
		return callback( this, other );
	}
	else 
	{
		// flip the types when looking into the index.
		int idx = myType * NUM_COLLIDER_TYPES + otherType;
		CollisionCheckCallback callback = g_CollisionChecks[idx];
		return callback( other, this );
	}
}


//-----------------------------------------------------------------------------------------------
static Manifold2 DiscVDiscCollisionManifoldGenerator( const Collider2D* collider1, const Collider2D* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider2D* discCollider1 = (const DiscCollider2D*)collider1;
	const DiscCollider2D* discCollider2 = (const DiscCollider2D*)collider2;
	
	Manifold2 manifold;
	manifold.normal = discCollider2->m_worldPosition - discCollider1->m_worldPosition;
	manifold.normal.Normalize();

	Vec2 disc1Edge = discCollider1->m_worldPosition + ( manifold.normal * discCollider1->m_radius );
	Vec2 disc2Edge = discCollider2->m_worldPosition + ( -manifold.normal * discCollider2->m_radius );
	manifold.penetrationDepth = GetDistance2D( disc1Edge, disc2Edge );

	manifold.contactPoint = disc1Edge - ( manifold.normal * manifold.penetrationDepth * .5f );

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static Manifold2 DiscVPolygonCollisionManifoldGenerator( const Collider2D* collider1, const Collider2D* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider2D* discCollider = (const DiscCollider2D*)collider1;
	const PolygonCollider2D* polygonCollider = (const PolygonCollider2D*)collider2;

	Vec2 closestPointOnPolygonToDisc = polygonCollider->m_polygon.GetClosestPointOnEdge( discCollider->m_worldPosition );
	
	Manifold2 manifold;
	manifold.normal = closestPointOnPolygonToDisc - discCollider->m_worldPosition;
	manifold.normal.Normalize();

	// If disc is inside polygon flip the normal to ensure it is pushed out
	if ( polygonCollider->m_polygon.Contains( discCollider->m_worldPosition ) )
	{
		manifold.normal *= -1.f;
	}

	Vec2 closestPointOnDiscToPolygon = discCollider->m_worldPosition + ( manifold.normal * discCollider->m_radius );
	manifold.penetrationDepth = GetDistance2D( closestPointOnDiscToPolygon, closestPointOnPolygonToDisc );

	manifold.contactPoint = closestPointOnDiscToPolygon - ( manifold.normal * manifold.penetrationDepth * .5f );

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static Manifold2 PolygonVPolygonCollisionManifoldGenerator( const Collider2D* collider1, const Collider2D* collider2 )
{
	UNUSED( collider1 );
	UNUSED( collider2 );
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	/*const PolygonCollider2D* polygonCollider1 = (const PolygonCollider2D*)collider1;
	const PolygonCollider2D* polygonCollider2 = (const PolygonCollider2D*)collider2;*/

	return Manifold2();
}


//-----------------------------------------------------------------------------------------------
// a "matrix" lookup is just a 2D array
static CollisionManifoldGenerationCallback g_ManifoldGenerators[NUM_COLLIDER_TYPES * NUM_COLLIDER_TYPES] = {
	/*             disc,                         polygon, */
	/*    disc */  DiscVDiscCollisionManifoldGenerator,      nullptr,
	/* polygon */  DiscVPolygonCollisionManifoldGenerator,   PolygonVPolygonCollisionManifoldGenerator
};


//-----------------------------------------------------------------------------------------------
Manifold2 Collider2D::GetCollisionManifold( const Collider2D* other ) const
{
	if ( other == nullptr
		 || !m_rigidbody->IsEnabled()
		 || !other->m_rigidbody->IsEnabled()
		 || !DoAABBsOverlap2D( GetWorldBounds(), other->GetWorldBounds() ) )
	{
		return Manifold2();
	}

	eCollider2DType myType = m_type;
	eCollider2DType otherType = other->m_type;

	if ( myType <= otherType )
	{
		int idx = otherType * NUM_COLLIDER_TYPES + myType;
		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
		return manifoldGenerator( this, other );
	}
	else
	{
		// flip the types when looking into the index.
		int idx = myType * NUM_COLLIDER_TYPES + otherType;
		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
		Manifold2 manifold = manifoldGenerator( other, this );
		manifold.normal *= -1.f;
		return manifold;
	}
}


//-----------------------------------------------------------------------------------------------
float Collider2D::GetBounceWith( const Collider2D* otherCollider ) const
{
	return m_material.m_bounciness * otherCollider->m_material.m_bounciness;
}


//-----------------------------------------------------------------------------------------------
float Collider2D::GetFrictionWith( const Collider2D* otherCollider ) const
{
	float combinedFriction = m_material.m_friction * otherCollider->m_material.m_friction;
	combinedFriction = ClampZeroToOne( combinedFriction );
	return combinedFriction;
}
