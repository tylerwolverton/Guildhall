#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct LineSegment2
{
public:
	Vec2 m_start = Vec2::ZERO;
	Vec2 m_end = Vec2::ZERO;

public:
	LineSegment2() = default;
	explicit LineSegment2( const Vec2& start, const Vec2& end );
	~LineSegment2();

	const Vec2 GetNearestPoint( const Vec2& point ) const;
	float GetOrientation() const;

	// Mutators
	void Translate( const Vec2& translation );
	void RotateDegrees( float displacementAngleDegrees );
};
