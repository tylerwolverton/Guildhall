#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct RaycastImpact
{
public:
	bool m_didImpact = false;
	Vec2 m_impactPosition = Vec2::ZERO;

public:
	RaycastImpact() {} // Do nothing default constructor
	explicit RaycastImpact( bool didImpact, const Vec2& impactPosition );
};
