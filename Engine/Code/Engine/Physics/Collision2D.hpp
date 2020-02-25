#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Manifold2.hpp"

class Collision2D
{
public:
	Collider2D* m_myCollider = nullptr;
	Collider2D* m_theirCollider = nullptr;
	Manifold2 m_collisionManifold;
};