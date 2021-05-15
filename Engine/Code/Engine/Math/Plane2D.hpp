#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct Plane2D
{
public:
	Vec2 normal;
	float distance;

public:
	Plane2D();
	Plane2D( const Vec2& normal, const Vec2& pointOnPlane );

	bool IsPointInFront( const Vec2& point ) const;
	float GetSignedDistanceFromPlane( const Vec2& point ) const;
};
