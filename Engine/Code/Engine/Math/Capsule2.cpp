#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Capsule2::Capsule2( const Vec2& middleStart, const Vec2& middleEnd, float radius )
	: m_middleStart( middleStart )
	, m_middleEnd( middleEnd )
	, m_radius( radius )
{
}


//-----------------------------------------------------------------------------------------------
Capsule2::~Capsule2()
{
}


//-----------------------------------------------------------------------------------------------
bool Capsule2::IsPointInside( const Vec2& point ) const
{
	return IsPointInsideCapsule2D( point, m_middleStart, m_middleEnd, m_radius );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Capsule2::GetNearestPoint( const Vec2& point ) const
{
	return GetNearestPointOnCapsule2D( point, m_middleStart, m_middleEnd, m_radius );
}


//-----------------------------------------------------------------------------------------------
float Capsule2::GetOrientation() const
{
	Vec2 middleSegment = m_middleEnd - m_middleStart;

	return middleSegment.GetOrientationDegrees();
}


//-----------------------------------------------------------------------------------------------
const OBB2 Capsule2::GetCenterSectionAsOBB2() const
{
	Vec2 capsuleCenter = ( m_middleStart + m_middleEnd ) * .5f;

	Vec2 capsuleMiddleSegment = m_middleEnd - m_middleStart;
	Vec2 fullDimensions( m_radius * 2.f, capsuleMiddleSegment.GetLength() );

	Vec2 iBasis = capsuleMiddleSegment.GetNormalized().GetRotated90Degrees();

	OBB2 centerObb( capsuleCenter, fullDimensions, iBasis );
	return centerObb;
}


//-----------------------------------------------------------------------------------------------
void Capsule2::Translate( const Vec2& translation )
{
	m_middleStart += translation;
	m_middleEnd += translation;
}


//-----------------------------------------------------------------------------------------------
void Capsule2::RotateDegrees( float displacementAngleDegrees )
{
	Vec2 center = ( m_middleStart + m_middleEnd ) * .5f;

	Translate( -center );

	m_middleStart.RotateDegrees( displacementAngleDegrees );
	m_middleEnd.RotateDegrees( displacementAngleDegrees );

	Translate( center );
}
