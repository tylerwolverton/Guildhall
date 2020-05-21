#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class DiscCollider2D : public Collider2D
{
public:
	DiscCollider2D( int id, const Vec2& localPosition, float radius );

	virtual void UpdateWorldShape() override;

	// queries 
	virtual const Vec2 GetClosestPoint( const Vec2& pos ) const override;
	virtual bool Contains( const Vec2& pos ) const override;

	virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const override;
	const AABB2 CalculateWorldBounds();

	virtual float CalculateMoment( float mass ) override;

	virtual Vec2 GetFarthestPointInDirection( const Vec2& direction ) const override;

	// debug helpers
	virtual void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const override;

protected:
	virtual ~DiscCollider2D();

public:
	float	m_radius = 0.f;
};
