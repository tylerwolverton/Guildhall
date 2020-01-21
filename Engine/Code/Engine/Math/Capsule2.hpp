#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct OBB2;


//-----------------------------------------------------------------------------------------------
struct Capsule2
{
public:
	Vec2 m_middleStart = Vec2::ZERO;
	Vec2 m_middleEnd = Vec2::ZERO;
	float m_radius = 0.f;

public:
	Capsule2() = default;
	explicit Capsule2( const Vec2& middleStart, const Vec2& middleEnd, float radius );
	~Capsule2();

	bool IsPointInside( const Vec2& point ) const;
	const Vec2 GetNearestPoint( const Vec2& point ) const;
	float GetOrientation() const;
	const OBB2 GetCenterSectionAsOBB2() const;

	// Mutators
	void Translate( const Vec2& translation );
	void RotateDegrees( float displacementAngleDegrees );
};