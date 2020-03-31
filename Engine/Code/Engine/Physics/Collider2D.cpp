#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Manifold2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Renderer/DebugRender.hpp"


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
static Vec2 GetSupportPoint( const PolygonCollider2D* polygonCollider1, const PolygonCollider2D* polygonCollider2, const Vec2& direction )
{
	return polygonCollider1->GetFarthestPointInDirection( direction ) - polygonCollider2->GetFarthestPointInDirection( -direction );
}


//-----------------------------------------------------------------------------------------------
static std::vector<Vec2> GetSimplexForGJKCollision( const PolygonCollider2D* polygonCollider1, const PolygonCollider2D* polygonCollider2 )
{
	// Initial point calculation
	Vec2 direction = polygonCollider2->m_worldPosition - polygonCollider1->m_worldPosition;
	Vec2 supportPoint0 = GetSupportPoint( polygonCollider1, polygonCollider2, direction );
	Vec2 supportPoint1 = GetSupportPoint( polygonCollider1, polygonCollider2, -direction );

	Vec2 supportEdge01 = supportPoint1 - supportPoint0;
	direction = TripleProduct2D( supportEdge01, -supportPoint0, supportEdge01 );

	while ( true )
	{
		Vec2 supportPoint2 = GetSupportPoint( polygonCollider1, polygonCollider2, direction );

		// If the new support point equals an existing one, we've hit the edge of the polygon so we know there is no intersection
		if ( supportPoint2 == supportPoint0
			 || supportPoint2 == supportPoint1 )
		{
			return std::vector<Vec2>{};
		}

		// Maintain ccw order
		if ( supportEdge01.GetRotated90Degrees().GetNormalized() != direction.GetNormalized() )
		{
			Vec2 temp = supportPoint0;
			supportPoint0 = supportPoint1;
			supportPoint1 = temp;
			supportEdge01 = supportPoint1 - supportPoint0;
		}

		// Get orthogonal vectors to each of the remaining edges to test for the origin
		Vec2 supportPoint2ToOrigin = -supportPoint2;
		Vec2 supportEdge21 = supportPoint1 - supportPoint2;
		Vec2 supportEdge20 = supportPoint0 - supportPoint2;

		Vec2 orthogonal21 = TripleProduct2D( supportEdge20, supportEdge21, supportEdge21 );
		Vec2 orthogonal20 = TripleProduct2D( supportEdge21, supportEdge20, supportEdge20 );

		// If origin is on opposite side of an edge, reset support points and try again
		if ( DotProduct2D( orthogonal21, supportPoint2ToOrigin ) > 0.f )
		{
			supportPoint0 = supportPoint1;
			supportPoint1 = supportPoint2;
			direction = orthogonal21;
			continue;
		}

		if ( DotProduct2D( orthogonal20, supportPoint2ToOrigin ) > 0.f )
		{
			supportPoint1 = supportPoint0;
			supportPoint0 = supportPoint2;
			direction = orthogonal20;
			continue;
		}

	//	return std::vector<Vec2>{ supportPoint0, supportPoint1, supportPoint2 };

		// Make sure simplex is in ccw order
		if ( orthogonal21.GetNormalized() != supportEdge21.GetRotated90Degrees().GetNormalized() )
		{
			return std::vector<Vec2>{ supportPoint0, supportPoint1, supportPoint2 };
		}

		return std::vector<Vec2>{ supportPoint2, supportPoint1, supportPoint0 };
	}

	return std::vector<Vec2>{};
}


//-----------------------------------------------------------------------------------------------
static bool PolygonVPolygonCollisionCheck( const Collider2D* collider1, const Collider2D* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const PolygonCollider2D* polygonCollider1 = (const PolygonCollider2D*)collider1;
	const PolygonCollider2D* polygonCollider2 = (const PolygonCollider2D*)collider2;

	std::vector<Vec2> simplex = GetSimplexForGJKCollision( polygonCollider1, polygonCollider2 );
	if ( simplex.size() == 0 )
	{
		return false;
	}

	return true;
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

	manifold.contactPoint1 = disc1Edge - ( manifold.normal * manifold.penetrationDepth * .5f );
	manifold.contactPoint2 = manifold.contactPoint1;

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

	manifold.contactPoint1 = closestPointOnDiscToPolygon - ( manifold.normal * manifold.penetrationDepth * .5f );
	manifold.contactPoint2 = manifold.contactPoint1;

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static Manifold2 PolygonVPolygonCollisionManifoldGenerator( const Collider2D* collider1, const Collider2D* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const PolygonCollider2D* polygonCollider1 = (const PolygonCollider2D*)collider1;
	const PolygonCollider2D* polygonCollider2 = (const PolygonCollider2D*)collider2;

	std::vector<Vec2> simplex = GetSimplexForGJKCollision( polygonCollider1, polygonCollider2 );
	if ( simplex.size() == 0 )
	{
		return Manifold2();
	}

	DebugAddWorldArrow( simplex[0], simplex[1], Rgba8::CYAN );
	DebugAddWorldArrow( simplex[1], simplex[2], Rgba8::MAGENTA );
	DebugAddWorldArrow( simplex[2], simplex[0], Rgba8::YELLOW );

	Vec2 edge01 = simplex[1] - simplex[0];
	if ( DotProduct2D( edge01.GetRotated90Degrees(), simplex[2] ) < 0.f )
	{
		Vec2 temp = simplex[0];
		simplex[0] = simplex[2];
		simplex[2] = temp;
	}

	DebugAddWorldArrow( simplex[0], simplex[1], Rgba8::RED );
	DebugAddWorldArrow( simplex[1], simplex[2], Rgba8::GREEN );
	DebugAddWorldArrow( simplex[2], simplex[0], Rgba8::BLUE );

	Polygon2 simplexPoly( simplex );
	for ( int vertexIdx = 0; vertexIdx < 32; ++vertexIdx )
	{
		Vec2 startEdge;
		Vec2 endEdge;
		simplexPoly.GetClosestEdge( Vec2::ZERO, &startEdge, &endEdge );
		Vec2 normal = ( endEdge - startEdge ).GetRotatedMinus90Degrees().GetNormalized();
		Vec2 support = GetSupportPoint( polygonCollider1, polygonCollider2, normal );

		Plane2D plane( normal, startEdge );

		if ( fabsf( DotProduct2D( support, normal ) - plane.distance ) <= .0001f )
		{
			Manifold2 manifold;
			manifold.normal = normal;
			manifold.penetrationDepth = plane.distance;
			manifold.contactPoint1 = startEdge;
			manifold.contactPoint2 = endEdge;
			return manifold;
		}
		else
		{
			std::vector<Vec2> newSimplex;
			for ( int vertIdx = 0; vertIdx < (int)simplex.size(); ++vertIdx )
			{
				newSimplex.push_back( simplex[vertIdx] );
				if ( simplex[vertIdx] == startEdge )
				{
					newSimplex.push_back( support );
				}
			}

			simplex = newSimplex;
			simplexPoly = Polygon2( newSimplex );
		}
	}

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
