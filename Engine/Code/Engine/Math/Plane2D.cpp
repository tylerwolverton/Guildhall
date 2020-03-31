#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Plane2D::Plane2D()
	: normal( 0, 1 )
	, distance( 0 ) 
{
}


//-----------------------------------------------------------------------------------------------
Plane2D::Plane2D( const Vec2& normal, const Vec2& pointOnPlane )
	: normal( normal )
	, distance( DotProduct2D( pointOnPlane, normal ) ) 
{
}


//-----------------------------------------------------------------------------------------------
bool Plane2D::GetPointIsInFront( const Vec2& point )
{
	float pointDistFromPlane = DotProduct2D( point, normal );
	float distanceFromPlane = pointDistFromPlane - distance;

	return distanceFromPlane > 0.0f;
}


//-----------------------------------------------------------------------------------------------
float Plane2D::GetSignedDistanceFromPlane( const Vec2& point )
{
	float pointDistFromPlane = DotProduct2D( point, normal );
	float distanceFromPlane = pointDistFromPlane - distance;

	return distanceFromPlane;
}
