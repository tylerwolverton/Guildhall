#include "Engine/Math/Polygon2.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Polygon2::Polygon2( const std::vector<Vec2>& points )
	: m_points( points )
{
}


//-----------------------------------------------------------------------------------------------
Polygon2::~Polygon2()
{
	m_points.clear();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetPoints( const std::vector<Vec2>& points )
{
	m_points.clear();

	m_points = points;
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::IsValid() const
{
	if ( m_points.size() < 3 )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::IsConvex() const
{
	if ( !IsValid() )
	{
		return false;
	}

	int totalNumPoints = (int)m_points.size();

	for ( int pointNumIdx = 0; pointNumIdx < totalNumPoints; ++pointNumIdx )
	{
		Vec2 pointA = m_points[pointNumIdx];
		Vec2 pointB;
		if ( pointNumIdx == totalNumPoints - 1 )
		{
			pointB = m_points[0];
		}
		else
		{
			int nextPointIdx = pointNumIdx + 1;
			pointB = m_points[nextPointIdx];
		}

		Vec2 edge = pointB - pointA;
		Vec2 normal = edge.GetRotated90Degrees();

		// TODO: Handle edge cases discussed in class
		if ( DotProduct2D( edge, normal ) < 0 )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::Contains( Vec2 point ) const
{
	return false;
}


//-----------------------------------------------------------------------------------------------
float Polygon2::GetDistance( Vec2 point ) const
{
	return 0.f;
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2::GetClosestPoint( Vec2 point ) const
{
	return point;
}


//-----------------------------------------------------------------------------------------------
int Polygon2::GetVertexCount() const
{
	return (int)m_points.size();
}


//-----------------------------------------------------------------------------------------------
int Polygon2::GetEdgeCount() const
{
	return (int)m_points.size() - 1;
}


//-----------------------------------------------------------------------------------------------
void Polygon2::GetEdge( Vec2* outStart, Vec2* outEnd )
{

}
