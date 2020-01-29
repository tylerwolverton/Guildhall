#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Vec2.hpp"

class DiscCollider2D : public Collider2D
{
public: // implement the interface of Collider2D
   // A02 TODO

public:
	Vec2	m_localPosition; // my local offset from my parent
	Vec2	m_worldPosition; // calculated from local position and owning rigidbody if present
	float	m_radius;
};
