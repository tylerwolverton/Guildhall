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
		Vec2 pointC;
		if ( pointNumIdx == totalNumPoints - 1 )
		{
			pointB = m_points[0];
			pointC = m_points[1];
		}
		else if( pointNumIdx == totalNumPoints - 2 )
		{
			int lastPointIdx = totalNumPoints - 1;
			pointB = m_points[lastPointIdx];
			pointC = m_points[0];
		}
		else
		{
			int nextPointIdx = pointNumIdx + 1;
			pointB = m_points[nextPointIdx];
			
			++nextPointIdx;
			pointC = m_points[nextPointIdx];
		}

		Vec2 normal = pointB - pointA;
		normal.Rotate90Degrees();

		Vec2 nextEdge = pointC - pointA;

		// TODO: Handle edge cases discussed in class
		if ( DotProduct2D( nextEdge, normal ) > 0 )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::Contains( Vec2 point ) const
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

		Vec2 normal = pointB - pointA;
		normal.Rotate90Degrees();

		Vec2 nextEdge = point - pointB;

		// TODO: Handle edge cases discussed in class
		if ( DotProduct2D( nextEdge, normal ) > 0 )
		{
			return false;
		}
	}

	return true;
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
	return (int)m_points.size();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::GetEdge( int edgeIndex, Vec2* outStart, Vec2* outEnd )
{

}


//-----------------------------------------------------------------------------------------------
void Polygon2::Translate2D( const Vec2& translation )
{
	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		m_points[pointNumIdx] += translation;
	}
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2::GetCenterOfMass() const
{
	Vec2 sumOfPoints( Vec2::ZERO );
	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		sumOfPoints += m_points[pointNumIdx];
	}

	return sumOfPoints / GetVertexCount();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetCenterOfMassAndUpdatePoints( const Vec2& newCenterOfMass )
{
	Vec2 oldCenterOfMass = GetCenterOfMass();

	Vec2 translation = newCenterOfMass - oldCenterOfMass;

	Translate2D( translation );
}
