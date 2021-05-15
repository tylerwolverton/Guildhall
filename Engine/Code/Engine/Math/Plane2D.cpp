#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Plane2D::Plane2D()
	: normal( 0.f, 1.f )
	, distance( 0.f ) 
{
}


//-----------------------------------------------------------------------------------------------
Plane2D::Plane2D( const Vec2& normal, const Vec2& pointOnPlane )
	: normal( normal )
	, distance( DotProduct2D( pointOnPlane, normal ) ) 
{
}


//-----------------------------------------------------------------------------------------------
bool Plane2D::IsPointInFront( const Vec2& point ) const
{
	float pointDistFromPlane = DotProduct2D( point, normal );
	float distanceFromPlane = pointDistFromPlane - distance;

	return distanceFromPlane > 0.0f;
}


//-----------------------------------------------------------------------------------------------
float Plane2D::GetSignedDistanceFromPlane( const Vec2& point ) const
{
	float pointDistFromPlane = DotProduct2D( point, normal );
	float distanceFromPlane = pointDistFromPlane - distance;

	return distanceFromPlane;
}
