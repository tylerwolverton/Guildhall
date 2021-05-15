#pragma once
#include "Engine/UI/UIElement.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class InputSystem;
class SpriteDefinition;
class Texture;
class UIPanel;
class UILabel;


//-----------------------------------------------------------------------------------------------
class UIUniformGrid : public UIElement
{
	friend class UIPanel;

public:
	virtual ~UIUniformGrid();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	std::vector<UIElement*> GetGridElements() const											{ return m_gridElements; }
	
private:
	UIUniformGrid( UISystem& uiSystem, const UIElement& parentPanel, const std::string& name, 
				   const UIAlignedPositionData& gridPositionData, const IntVec2& gridDimensions, const Vec2& paddingOfGridElementsPixels = Vec2::ZERO, 
				   Texture* elementTexture = nullptr, const Rgba8& elementTint = Rgba8::WHITE );

	// Events
	void OnElementHoverBegin( EventArgs* args );
	void OnElementHoverEnd( EventArgs* args );

private:
	int						m_gridCapacity = 0;
	std::vector<UIElement*> m_gridElements;
};
