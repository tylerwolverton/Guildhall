#pragma once
#include "Engine/Math/Vec2.hpp"


class Physics2D;
class Collider2D;


class Rigidbody2D
{
	friend class Physics2D;

public:
	void Destroy(); // helper for destroying myself (uses owner to destroy self)

	void TakeCollider( Collider2D* collider ); // takes ownership of a collider (destroying my current one if present)

public:
	Physics2D* m_system;     // which scene created/owns this object
	Collider2D* m_collider;

	Vec2 m_worldPosition;   // where in the world is this rigidbody

private:
	~Rigidbody2D(); // destroys the collider
};
