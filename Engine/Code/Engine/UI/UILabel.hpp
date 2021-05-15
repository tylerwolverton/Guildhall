#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/UI/UIElement.hpp"


//-----------------------------------------------------------------------------------------------
struct UIAlignedPositionData;
struct UIRelativePositionData;
class UIElement;
class UISystem;


//-----------------------------------------------------------------------------------------------
class UILabel : public UIElement
{
public:
	virtual ~UILabel() {}

	virtual void Update(){}
	virtual void Render() const = 0;
	virtual void DebugRender() const;

	virtual AABB2 GetBounds() const											{ return m_boundingBox; }
	virtual void SetTint( const Rgba8& tint )								{ m_initialTint = tint; }

protected:
	UILabel( const std::string& name, UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData );
	UILabel( const std::string& name, UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData );

protected:
	AABB2 m_boundingBox;
	Rgba8 m_initialTint = Rgba8::WHITE;
};
