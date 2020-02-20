#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
static bool DiscVDiscCollisionCheck( const Collider2D* col0, const Collider2D* col1 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider2D* disc0 = (const DiscCollider2D*)col0;
	const DiscCollider2D* disc1 = (const DiscCollider2D*)col1;

	return DoDiscsOverlap( disc0->m_worldPosition, 
						   disc0->m_radius, 
						   disc1->m_worldPosition,
						   disc1->m_radius );
}


//-----------------------------------------------------------------------------------------------
static bool DiscVPolygonCollisionCheck( const Collider2D* col0, const Collider2D* col1 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider2D* disc = (const DiscCollider2D*)col0;
	const PolygonCollider2D* polygon = (const PolygonCollider2D*)col1;

	Vec2 nearestPoint = polygon->GetClosestPoint( disc->m_worldPosition );
	return IsPointInsideDisc( nearestPoint, disc->m_worldPosition, disc->m_radius );
}


//-----------------------------------------------------------------------------------------------
static bool PolygonVPolygonCollisionCheck( const Collider2D* col0, const Collider2D* col1 )
{
	UNUSED( col0 );
	UNUSED( col1 );
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	/*const PolygonCollider2D* polygon0 = (const PolygonCollider2D*)col0;
	const PolygonCollider2D* polygon1 = (const PolygonCollider2D*)col1;*/

	return false;
}


//-----------------------------------------------------------------------------------------------
// a "matrix" lookup is just a 2D array
static collision_check_cb gCollisionChecks[NUM_COLLIDER_TYPES * NUM_COLLIDER_TYPES] = {
	/*             disc,                         polygon, */
	/*    disc */  DiscVDiscCollisionCheck,      nullptr,
	/* polygon */  DiscVPolygonCollisionCheck,   PolygonVPolygonCollisionCheck
};


//-----------------------------------------------------------------------------------------------
bool Collider2D::Intersects( const Collider2D* other ) const
{
	if ( !DoAABBsOverlap2D( GetBoundingBox(), other->GetBoundingBox() ) )
	{
		return false;
	}

	eCollider2DType myType = m_type;
	eCollider2DType otherType = other->m_type;

	if ( myType <= otherType ) 
	{
		int idx = otherType * NUM_COLLIDER_TYPES + myType;
		collision_check_cb check = gCollisionChecks[idx];
		return check( this, other );
	}
	else 
	{
		// flip the types when looking into the index.
		int idx = myType * NUM_COLLIDER_TYPES + otherType;
		collision_check_cb check = gCollisionChecks[idx];
		return check( other, this );
	}
}
