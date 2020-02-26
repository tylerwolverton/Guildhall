#pragma once
#include "Engine/Math/Vec2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Polygon2
{
public:
	Polygon2() = default;
	Polygon2( const std::vector<Vec2>& points );
	~Polygon2();

	void SetPoints( const std::vector<Vec2>& points );
	std::vector<Vec2> GetPoints() const											{ return m_points; };

	bool IsValid() const; // must have at least 3 points to be considered a polygon

	bool IsConvex() const;
	bool Contains( Vec2 point ) const;
	float GetDistance( Vec2 point ) const;
	Vec2 GetClosestPoint( Vec2 point ) const;
	Vec2 GetClosestPointOnEdge( Vec2 point ) const;

	// accessors
	int GetVertexCount() const;
	int GetEdgeCount() const;
	void GetEdge( int edgeIndex, Vec2* out_start, Vec2* out_end ) const;

	void Translate2D( const Vec2& translation );

	Vec2 GetCenterOfMass() const;
	void SetCenterOfMassAndUpdatePoints( const Vec2& newCenterOfMass );

public:
	// Stored in counter clockwise order
	std::vector<Vec2> m_points;
};
