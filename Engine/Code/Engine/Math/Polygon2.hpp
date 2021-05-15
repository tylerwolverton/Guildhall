#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Plane2D;
class ConvexHull2D;


//-----------------------------------------------------------------------------------------------
class Polygon2
{
	friend class PolygonCollider2D;

public:
	Polygon2() = default;
	Polygon2( const std::vector<Vec2>& points );
	~Polygon2();

	void SetPoints( Vec2* points, int numPoints );
	void SetPoints( const std::vector<Vec2>& points );
	std::vector<Vec2> GetPoints() const											{ return m_points; };

	bool IsValid() const; // must have at least 3 points to be considered a polygon

	bool IsConvex() const;
	bool Contains( const Vec2& point ) const;
	float GetDistance( const Vec2& point ) const;
	Vec2 GetClosestPoint( const Vec2& point ) const;
	Vec2 GetClosestPointOnEdge( const Vec2& point ) const;
	void GetClosestEdge( const Vec2& point, Vec2* out_start, Vec2* out_end ) const;

	ConvexHull2D GenerateConvexHull() const;

	// accessors
	int GetVertexCount() const;
	int GetEdgeCount() const;
	void GetEdge( int edgeIndex, Vec2* out_start, Vec2* out_end ) const;

	void Translate2D( const Vec2& translation );
	void TranslateWithBoundingBox2D( const Vec2& translation );
	void SetOrientation( float newOrientationDegrees );

	Vec2 GetCenterOfMass() const;
	void SetCenterOfMass( const Vec2& newCenterOfMass )											{ m_centerOfMass = newCenterOfMass; }
	void SetCenterOfMassAndUpdatePoints( const Vec2& newCenterOfMass );

	void RotateAboutPoint2D( float rotationDegrees, const Vec2& rotationPoint );
	void ScaleAboutPoint2D( float scaleFactor, const Vec2& scaleOriginPoint );

private:
	Plane2D GenerateOutwardFacingPlaneFromEdge( const Vec2& startPoint, const Vec2& endPoint ) const;

	void CalculateBoundingBox();
	void Rotate2D( float rotationDegrees );

private:
	// Stored in counter clockwise order
	std::vector<Vec2> m_points;
	
	// TODO: Probably best to move this stuff to a physics poly or something to keep this class lightweight
	float m_orientationDegrees = 0.f;
	AABB2 m_boundingBox;

	Vec2 m_centerOfMass = Vec2::ZERO;
};
