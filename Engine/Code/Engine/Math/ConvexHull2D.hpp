#pragma once
#include "Engine/Math/Plane2D.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class RenderContext;


//-----------------------------------------------------------------------------------------------
struct RayConvexHullIntersectionResult
{
public:
	Vec2 intersectionPoint = Vec2::ZERO;
	Vec2 surfaceNormal = Vec2::ZERO;

public:
	RayConvexHullIntersectionResult( const Vec2& intersectionPoint, const Vec2& surfaceNormal )
		: intersectionPoint( intersectionPoint )
		, surfaceNormal( surfaceNormal )
	{}
};


//-----------------------------------------------------------------------------------------------
class ConvexHull2D
{
public:
	ConvexHull2D() {}
	~ConvexHull2D() {}

	void AddPlane( const Plane2D& newPlane );

	bool IsPointInside( const Vec2& point );

	RayConvexHullIntersectionResult GetRayIntersectionResult( const Vec2& rayStartPos, const Vec2& rayForwardNormal );

	void DebugRender( RenderContext* renderer, const Vec2& worldPosition ) const;

private:
	std::vector<Plane2D> m_boundingPlanes;
};
