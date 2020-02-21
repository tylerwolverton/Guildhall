#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct Manifold2
{
	Vec2 normal = Vec2::ZERO;
	float penetrationDepth = 0.f;
};
