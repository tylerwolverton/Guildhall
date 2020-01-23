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
	//IsPointInside( const Vec2& point );

private:
	// Stored in counter clockwise order
	std::vector<Vec2> m_points;
};
