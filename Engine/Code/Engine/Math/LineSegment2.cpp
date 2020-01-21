#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
LineSegment2::LineSegment2( const Vec2& start, const Vec2& end )
	: m_start( start )
	, m_end( end )
{
}


//-----------------------------------------------------------------------------------------------
LineSegment2::~LineSegment2()
{
}


//-----------------------------------------------------------------------------------------------
const Vec2 LineSegment2::GetNearestPoint( const Vec2& point ) const
{
	return GetNearestPointOnLineSegment2D( point, m_start, m_end );
}


//-----------------------------------------------------------------------------------------------
float LineSegment2::GetOrientation() const
{
	Vec2 directedSegment = m_end - m_start;

	return directedSegment.GetOrientationDegrees();
}


//-----------------------------------------------------------------------------------------------
void LineSegment2::Translate( const Vec2& translation )
{
	m_start += translation;
	m_end += translation;
}


//-----------------------------------------------------------------------------------------------
void LineSegment2::RotateDegrees( float displacementAngleDegrees )
{
	Vec2 center = ( m_start + m_end ) * .5f;

	Translate( -center );

	m_start.RotateDegrees( displacementAngleDegrees );
	m_end.RotateDegrees( displacementAngleDegrees );

	Translate( center );
}
