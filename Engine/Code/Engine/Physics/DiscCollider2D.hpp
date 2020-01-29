#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class DiscCollider2D : public Collider2D
{
public:
	DiscCollider2D();

	virtual void UpdateWorldShape() const override;

	// queries 
	virtual const Vec2 GetClosestPoint( const Vec2& pos ) const override;
	virtual bool Contains( const Vec2& pos ) const override;
	virtual bool Intersects( const Collider2D* other ) const override;

	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) override;

private:
	virtual ~DiscCollider2D();

public:
	Vec2	m_localPosition; // my local offset from my parent
	Vec2	m_worldPosition; // calculated from local position and owning rigidbody if present
	float	m_radius = 0.f;
};
