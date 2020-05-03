#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Manifold2.hpp"


//-----------------------------------------------------------------------------------------------
class Collision2D
{
public:
	IntVec2 id = IntVec2( -1, -1 );
	uint frameNum = 0;

	Collider2D* myCollider = nullptr;
	Collider2D* theirCollider = nullptr;
	Manifold2 collisionManifold;
};
