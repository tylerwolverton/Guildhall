#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <math.h>

//-----------------------------------------------------------------------------------------------
OBB2::OBB2( const OBB2& copyFrom )
	: m_center( copyFrom.GetCenter() )
	, m_halfDimensions( copyFrom.GetHalfDimensions() )
	, m_iBasis( copyFrom.GetIBasisNormal() )
{
}


//-----------------------------------------------------------------------------------------------
OBB2::OBB2( const Vec2& center, const Vec2& fullDimensions, const Vec2& iBasisNormal /*= Vec2( 1.f, 0.f ) */ )
	: m_center( center )
	, m_halfDimensions( fullDimensions * .5f )
	, m_iBasis( iBasisNormal )
{
}


//-----------------------------------------------------------------------------------------------
OBB2::OBB2( const Vec2& center, const Vec2& fullDimensions, float orientationDegrees )
	: m_center( center )
	, m_halfDimensions( fullDimensions * .5f )
{
	m_iBasis = Vec2::MakeFromPolarDegrees( orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
OBB2::OBB2( const AABB2& asAxisAlignedBox, float orientationDegrees /*= 0.f */ )
{
	Vec2 aabb2Dimensions = asAxisAlignedBox.maxs - asAxisAlignedBox.mins;
	
	m_halfDimensions = aabb2Dimensions * .5f;
	m_center = asAxisAlignedBox.mins + m_halfDimensions;
	m_iBasis = Vec2::MakeFromPolarDegrees( orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
bool OBB2::IsPointInside( const Vec2& point ) const
{
	Vec2 displacementWorldCoords = point - m_center; 

	float pointLocali = DotProduct2D( displacementWorldCoords, m_iBasis );
	float pointLocalj = DotProduct2D( displacementWorldCoords, GetJBasisNormal() );

	return  ( pointLocali < m_halfDimensions.x
			  && pointLocali > -m_halfDimensions.x
			  && pointLocalj < m_halfDimensions.y
			  && pointLocalj > -m_halfDimensions.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 OBB2::GetJBasisNormal() const
{
	return Vec2( -m_iBasis.y, m_iBasis.x );
}


//-----------------------------------------------------------------------------------------------
float OBB2::GetOrientationDegrees() const
{
	return m_iBasis.GetOrientationDegrees();
}


//-----------------------------------------------------------------------------------------------
const Vec2 OBB2::GetNearestPoint( const Vec2& point ) const
{
	Vec2 displacementWorldCoords = point - m_center;

	float pointLocali = DotProduct2D( displacementWorldCoords, m_iBasis );
	float pointLocalj = DotProduct2D( displacementWorldCoords, GetJBasisNormal() );

	float newLocalX = ClampMinMax( pointLocali, -m_halfDimensions.x, m_halfDimensions.x );
	float newLocalY = ClampMinMax( pointLocalj, -m_halfDimensions.y, m_halfDimensions.y );

	Vec2 nearestPoint( m_center );
	nearestPoint += ( newLocalX * m_iBasis );
	nearestPoint += ( newLocalY * GetJBasisNormal() );

	return nearestPoint;
}


//-----------------------------------------------------------------------------------------------
const Vec2 OBB2::GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const
{
	float mappedX = RangeMapFloat( 0.f, 1.f, -m_halfDimensions.x, m_halfDimensions.x, uvCoordsZeroToOne.x );
	float mappedY = RangeMapFloat( 0.f, 1.f, -m_halfDimensions.y, m_halfDimensions.y, uvCoordsZeroToOne.y );

	Vec2 point( m_center );
	point += ( mappedX * m_iBasis );
	point += ( mappedY * GetJBasisNormal() );

	return point;
}


//-----------------------------------------------------------------------------------------------
const Vec2 OBB2::GetUVForPoint( const Vec2& point ) const
{
	Vec2 displacementWorldCoords = point - m_center;

	float pointLocali = DotProduct2D( displacementWorldCoords, m_iBasis );
	float pointLocalj = DotProduct2D( displacementWorldCoords, GetJBasisNormal() );

	float mappedX = RangeMapFloat( -m_halfDimensions.x, m_halfDimensions.x , 0.f, 1.f, pointLocali );
	float mappedY = RangeMapFloat( -m_halfDimensions.y, m_halfDimensions.y , 0.f, 1.f, pointLocalj );
	
	return Vec2( mappedX, mappedY );
}


//-----------------------------------------------------------------------------------------------
float OBB2::GetOuterRadius() const
{
	return m_halfDimensions.x + m_halfDimensions.y;
}


//-----------------------------------------------------------------------------------------------
float OBB2::GetInnerRadius() const
{

	if ( m_halfDimensions.x < m_halfDimensions.y )
	{
		return m_halfDimensions.x;
	}
	else
	{
		return m_halfDimensions.y;
	}
}


//-----------------------------------------------------------------------------------------------
void OBB2::GetCornerPositions( Vec2* out_fourPoints ) const
{
	Vec2 boxHalfWidth( m_halfDimensions.x * m_iBasis );
	Vec2 boxHalfHeight( m_halfDimensions.y * GetJBasisNormal() );

	Vec2 topRight( m_center + boxHalfWidth + boxHalfHeight );
	Vec2 topLeft( m_center - boxHalfWidth + boxHalfHeight );
	Vec2 bottomLeft( m_center - boxHalfWidth - boxHalfHeight );
	Vec2 bottomRight( m_center + boxHalfWidth - boxHalfHeight );

	out_fourPoints[0] = bottomLeft;
	out_fourPoints[1] = bottomRight;
	out_fourPoints[2] = topLeft;
	out_fourPoints[3] = topRight;
}


//-----------------------------------------------------------------------------------------------
void OBB2::Translate( const Vec2& translation )
{
	m_center += translation;
}


//-----------------------------------------------------------------------------------------------
void OBB2::SetCenter( const Vec2& newCenter )
{
	m_center = newCenter;
}


//-----------------------------------------------------------------------------------------------
void OBB2::SetDimensions( const Vec2& newDimensions )
{
	m_halfDimensions = newDimensions * .5f;
}


//-----------------------------------------------------------------------------------------------
void OBB2::SetOrientationDegrees( float newAbsoluteOrientation )
{
	m_iBasis = Vec2::MakeFromPolarDegrees( newAbsoluteOrientation );
}


//-----------------------------------------------------------------------------------------------
void OBB2::RotateByDegrees( float relativeRotationDegrees )
{
	float newAngleDegrees = m_iBasis.GetOrientationDegrees();
	newAngleDegrees += relativeRotationDegrees;

	m_iBasis = Vec2::MakeFromPolarDegrees( newAngleDegrees );
}


//-----------------------------------------------------------------------------------------------
void OBB2::StretchToIncludePoint( const Vec2& point )
{
	Vec2 localPoint = GetPointInLocalSpaceFromWorldSpace( point );

	Vec2 maxs = m_center + m_halfDimensions;
	Vec2 mins = m_center - m_halfDimensions;

	if( localPoint.x > maxs.x )
	{
		m_halfDimensions.x = localPoint.x - m_center.x;
	}
	else if ( localPoint.x < mins.x )
	{
		m_halfDimensions.x = localPoint.x - m_center.x;
	}

	if ( localPoint.y > maxs.y )
	{
		m_halfDimensions.y = localPoint.y - m_center.y;
	}
	else if ( localPoint.y < mins.y )
	{
		m_halfDimensions.y = localPoint.y - m_center.y;
	}

	Fix();
}


//-----------------------------------------------------------------------------------------------
void OBB2::Fix()
{
	// Ensure dimensions aren't negative
	m_halfDimensions.x = fabsf( m_halfDimensions.x );
	m_halfDimensions.y = fabsf( m_halfDimensions.y );

	// Normalize i basis
	if(m_iBasis == Vec2::ZERO)
	{
		m_iBasis = Vec2( 1.f, 0.f );
	}
	else
	{
		m_iBasis.Normalize();
	}
}


//-----------------------------------------------------------------------------------------------
const Vec2 OBB2::GetPointInLocalSpaceFromWorldSpace( const Vec2& point )
{
	Vec2 displacementWorldCoords = point - m_center;

	float pointLocali = DotProduct2D( displacementWorldCoords, m_iBasis );
	float pointLocalj = DotProduct2D( displacementWorldCoords, GetJBasisNormal() );

	return Vec2( pointLocali, pointLocalj );
}


//-----------------------------------------------------------------------------------------------
void OBB2::operator=( const OBB2& assignFrom )
{
	m_center = assignFrom.GetCenter();
	m_halfDimensions = assignFrom.GetHalfDimensions();
	m_iBasis = assignFrom.GetIBasisNormal();
}
