#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct Manifold2
{
	Vec2 normal = Vec2::ZERO;
	float penetrationDepth = 0.f;
	Vec2 contactPoint1 = Vec2::ZERO;
	Vec2 contactPoint2 = Vec2::ZERO;
};
