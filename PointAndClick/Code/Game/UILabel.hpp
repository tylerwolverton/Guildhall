#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class RenderContext;
class UIButton;


//-----------------------------------------------------------------------------------------------
class UILabel
{
public:
	UILabel( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions );

	virtual void Render( RenderContext* renderer ) = 0;

protected:
	AABB2 m_boundingBox;
};
