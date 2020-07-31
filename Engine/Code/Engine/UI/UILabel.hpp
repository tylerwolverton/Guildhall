#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
struct UIAlignedPositionData;
struct UIRelativePositionData;
class UIElement;
class UISystem;


//-----------------------------------------------------------------------------------------------
class UILabel
{
public:
	UILabel( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData );
	UILabel( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData );

	virtual void Render() const = 0;
	virtual void DebugRender() const;

	virtual void SetTint( const Rgba8& tint )								{ m_tint = tint; }

protected:
	const UISystem& m_uiSystem;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;
};
