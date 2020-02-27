#include "Engine/Math/Polygon2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Polygon2::Polygon2( const std::vector<Vec2>& points )
	: m_points( points )
{
	CalculateCenterOfMass();
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
		if ( DotProduct2D( nextEdge, normal ) < 0 )
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
		if ( DotProduct2D( nextEdge, normal ) < 0 )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
float Polygon2::GetDistance( Vec2 point ) const
{
	if ( Contains( point ) )
	{
		return 0.f;
	}

	float minDistToPoint = 99999999.f;
	for ( int edgeIdx = 0; edgeIdx < GetEdgeCount(); ++edgeIdx )
	{
		Vec2 startVert;
		Vec2 endVert;
		GetEdge( edgeIdx, &startVert, &endVert );

		Vec2 nearestLocalPoint = GetNearestPointOnLineSegment2D( point, startVert, endVert );
		float distToPoint = GetDistance2D( nearestLocalPoint, point );

		if ( distToPoint < minDistToPoint )
		{
			minDistToPoint = distToPoint;
		}
	}

	return minDistToPoint;
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2::GetClosestPoint( Vec2 point ) const
{
	if ( Contains( point ) )
	{
		return point;
	}

	return GetClosestPointOnEdge( point );
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2::GetClosestPointOnEdge( Vec2 point ) const
{
	Vec2 nearestPoint = Vec2::ZERO;
	float minDistToPoint = 99999999.f;
	for ( int edgeIdx = 0; edgeIdx < GetEdgeCount(); ++edgeIdx )
	{
		Vec2 startVert;
		Vec2 endVert;
		GetEdge( edgeIdx, &startVert, &endVert );

		Vec2 nearestLocalPoint = GetNearestPointOnLineSegment2D( point, startVert, endVert );
		float distToPoint = GetDistance2D( nearestLocalPoint, point );

		if ( distToPoint < minDistToPoint )
		{
			minDistToPoint = distToPoint;
			nearestPoint = nearestLocalPoint;
		}
	}

	return nearestPoint;
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
void Polygon2::GetEdge( int edgeIndex, Vec2* out_start, Vec2* out_end ) const
{
	if ( edgeIndex >= GetEdgeCount() 
		 || edgeIndex < 0 )
	{
		out_start = nullptr;
		out_end = nullptr;
		return;
	}

	int firstVertIdx = edgeIndex - 1;
	int secondVertIdx = edgeIndex;

	if ( firstVertIdx == -1 )
	{
		firstVertIdx = GetVertexCount() - 1;
	}

	if ( secondVertIdx == GetVertexCount() )
	{
		secondVertIdx = 0;
	}

	*out_start = m_points[firstVertIdx];
	*out_end = m_points[secondVertIdx];
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
// TODO: Calculate more accurately
Vec2 Polygon2::GetCenterOfMass() const
{
	return m_boundingBox.GetCenter();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetCenterOfMassAndUpdatePoints( const Vec2& newCenterOfMass )
{
	Vec2 oldCenterOfMass = GetCenterOfMass();

	Vec2 translation = newCenterOfMass - oldCenterOfMass;

	Translate2D( translation );

	m_boundingBox.SetCenter( newCenterOfMass );
}


//-----------------------------------------------------------------------------------------------
void Polygon2::CalculateCenterOfMass()
{
	// Initialize with first point in polygon
	AABB2 boundingBox( m_points[0], m_points[0] );

	for ( int pointIdx = 1; pointIdx < (int)m_points.size(); ++pointIdx )
	{
		boundingBox.StretchToIncludePoint( m_points[pointIdx] );
	}

	m_boundingBox = boundingBox;
}
