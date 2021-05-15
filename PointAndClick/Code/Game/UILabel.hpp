#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class RenderContext;
class UIElement;


//-----------------------------------------------------------------------------------------------
class UILabel
{
public:
	UILabel( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions );

	virtual void Render( RenderContext* renderer ) const = 0;
	virtual void DebugRender( RenderContext* renderer ) const;

	virtual void SetTint( const Rgba8& tint )								{ m_tint = tint; }

protected:
	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;
};
