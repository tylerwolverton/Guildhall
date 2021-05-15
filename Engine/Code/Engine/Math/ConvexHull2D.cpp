#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


//-----------------------------------------------------------------------------------------------
void ConvexHull2D::AddPlane( const Plane2D& newPlane )
{
	m_boundingPlanes.push_back( newPlane );
}


//-----------------------------------------------------------------------------------------------
bool ConvexHull2D::IsPointInside( const Vec2& point )
{
	for ( int planeNum = 0; planeNum < (int)m_boundingPlanes.size(); ++planeNum )
	{
		Plane2D& boundingPlane = m_boundingPlanes[planeNum];

		if ( boundingPlane.IsPointInFront( point ) )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
RayConvexHullIntersectionResult ConvexHull2D::GetRayIntersectionResult( const Vec2& rayStartPos, const Vec2& rayForwardNormal )
{
	// Find the furthest entry point on convex hull, since the entry will be the latest instance
	float entryPointDist = -9999999.f;
	Vec2 furthestEntryPointAlongRay( rayStartPos );
	Vec2 surfaceNormalOfIntersectionPlane = Vec2::ZERO;

	for ( int planeIdx = 0; planeIdx < (int)m_boundingPlanes.size(); ++planeIdx )
	{
		Plane2D const& boundingPlane = m_boundingPlanes[planeIdx];

		// Ignore this plane if it faces away from ray
		if ( DotProduct2D( rayForwardNormal, boundingPlane.normal ) > 0.f )
		{
			continue;
		}

		Vec2 intersectionPoint = GetRayIntersectionPointWithPlane2D( rayStartPos, rayForwardNormal, boundingPlane );
		// Ignore this plane if ray doesn't hit it
		if ( IsNearlyEqual( intersectionPoint, rayStartPos ) )
		{
			continue;
		}

		// Ignore this point if it's outside the hull
		if ( !IsPointInside( intersectionPoint + ( rayForwardNormal * .0001f ) ) )
		{
			continue;
		}

		// This is a potential entry point, check if it's further along the ray than current candidate
		float distToIntersectionPoint = GetDistanceSquared2D( rayStartPos, intersectionPoint );
		if ( distToIntersectionPoint > entryPointDist )
		{
			entryPointDist = distToIntersectionPoint;
			furthestEntryPointAlongRay = intersectionPoint;
			surfaceNormalOfIntersectionPlane = boundingPlane.normal;
		}
	}
	
	return RayConvexHullIntersectionResult( furthestEntryPointAlongRay, surfaceNormalOfIntersectionPlane );
}


//-----------------------------------------------------------------------------------------------
void ConvexHull2D::DebugRender( RenderContext* renderer, const Vec2& worldPosition ) const
{
	for ( int planeIdx = 0; planeIdx < (int)m_boundingPlanes.size(); ++planeIdx )
	{
		Plane2D const& boundingPlane = m_boundingPlanes[planeIdx];

		Vec2 tangent = boundingPlane.normal.GetRotatedMinus90Degrees();

		Vec2 pointOnPlane = boundingPlane.normal * boundingPlane.distance;

		DrawLine2D( renderer, pointOnPlane + tangent * -20.f, pointOnPlane + tangent * 20.f, Rgba8::DARK_YELLOW, .02f );

		Vec2 normalEndPoint = worldPosition + boundingPlane.normal * 1.f;
		DrawLine2D( renderer, worldPosition, normalEndPoint, Rgba8::MAGENTA, .02f );
		DrawLine2D( renderer, normalEndPoint, normalEndPoint + boundingPlane.normal.GetRotatedDegrees(120.f) * .05f, Rgba8::MAGENTA, .02f );
		DrawLine2D( renderer, normalEndPoint, normalEndPoint + boundingPlane.normal.GetRotatedDegrees(-120.f) * .05f, Rgba8::MAGENTA, .02f );
	}
}


