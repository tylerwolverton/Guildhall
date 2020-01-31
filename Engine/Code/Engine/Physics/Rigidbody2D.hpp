#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct Rgba8;
class Physics2D;
class Collider2D;
class RenderContext;


//-----------------------------------------------------------------------------------------------
class Rigidbody2D
{
	friend class Physics2D;

public:
	Rigidbody2D();

	void Destroy(); // helper for destroying myself (uses owner to destroy self)

	void TakeCollider( Collider2D* collider ); // takes ownership of a collider (destroying my current one if present)

	Vec2 GetPosition()																{ return m_worldPosition; }
	void SetPosition( const Vec2& position);

	void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const;

public:
	Collider2D* m_collider = nullptr;

private:
	Physics2D* m_system = nullptr;			// which scene created/owns this object
	Vec2 m_worldPosition = Vec2::ZERO;		// where in the world is this rigidbody

private:
	~Rigidbody2D(); // destroys the collider
};
