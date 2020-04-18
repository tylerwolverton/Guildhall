#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Polygon2
{
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

	// accessors
	int GetVertexCount() const;
	int GetEdgeCount() const;
	void GetEdge( int edgeIndex, Vec2* out_start, Vec2* out_end ) const;

	void Translate2D( const Vec2& translation );
	void SetOrientation( float newOrientationDegrees );

	Vec2 GetCenterOfMass() const;
	void SetCenterOfMassAndUpdatePoints( const Vec2& newCenterOfMass );

private:
	void CalculateBoundingBox();
	void Rotate2D( float rotationDegrees );

public:
	// Stored in counter clockwise order
	std::vector<Vec2> m_points;
	float m_orientationDegrees = 0.f;
	AABB2 m_boundingBox;

	Vec2 m_centerOfMass = Vec2::ZERO;
};
