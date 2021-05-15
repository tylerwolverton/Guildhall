#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class BitmapFont;
class Material;
class InputSystem;
class RenderContext;
class UIPanel;
class UIElement;
class Window;


//-----------------------------------------------------------------------------------------------
// Define the size and alignment of an element within its parent
//-----------------------------------------------------------------------------------------------
struct UIAlignedPositionData
{
public:
	Vec2 fractionOfParentDimensions = Vec2::ONE;
	Vec2 horizontalPaddingInPixels = Vec2::ZERO;
	Vec2 verticalPaddingInPixels = Vec2::ZERO;
	Vec2 positionOffsetFraction = Vec2::ZERO;
	Vec2 positionOffsetInPixels = Vec2::ZERO;
	Vec2 alignmentWithinParentElement = ALIGN_CENTERED;
};


//-----------------------------------------------------------------------------------------------
// Define the relative location of an element within its parent
//-----------------------------------------------------------------------------------------------
struct UIRelativePositionData
{
public:
	Vec2 widthFractionRange = Vec2::ZERO_TO_ONE;
	Vec2 heightFractionRange = Vec2::ZERO_TO_ONE;
	Vec2 horizontalPaddingInPixels = Vec2::ZERO;
	Vec2 verticalPaddingInPixels = Vec2::ZERO;
	Vec2 positionOffsetFraction = Vec2::ZERO;
	Vec2 positionOffsetInPixels = Vec2::ZERO;
};


//-----------------------------------------------------------------------------------------------
class UISystem
{
	friend class UIButton;
	friend class UIElement;
	friend class UIPanel;
	friend class UIButton;
	friend class UIUniformGrid;
	friend class UILabel;
	friend class UIImage;
	friend class UIText;

public:
	void Startup( Window* window, RenderContext* renderer, BitmapFont* font = nullptr, Material* material = nullptr );
	void Startup( const Vec2& windowDimensions, RenderContext* renderer ); // For testing
	void Update();
	void Render();
	void DebugRender();
	void Shutdown();

	UIPanel* const GetRootPanel()										{ return m_rootPanel; }

	void LoadUIElementsFromXML( const XmlElement& uiRootElem );

	UIElement* GetUIElementByName( const std::string& elemName );
	UIElement* GetUIElementById( int elemId );

	bool IsMouseCursorInUI() const;

	void SetFont( BitmapFont* font, Material* material = nullptr );

private:
	AABB2 GetBoundingBoxFromParentAndPositionData( const AABB2& parentBoundingBox, const UIAlignedPositionData& positionData ) const;
	AABB2 GetBoundingBoxFromParentAndPositionData( const AABB2& parentBoundingBox, const UIRelativePositionData& positionData ) const;

	void PadBoundingBox( AABB2& boundingBox, const Vec2& horizontalPaddingInPixels, const Vec2& verticalPaddingInPixels ) const;
	void OffsetBoundingBox( AABB2& boundingBox, const AABB2& parentBoundingBox, const Vec2& offsetFraction, const Vec2& offsetInPixels ) const;

	void RegisterUIElement( uint id, UIElement* uiElement );
	void RegisterUIElementName( const std::string& name, uint id );

	BitmapFont* GetFont() const												{ return m_uiFont; }
	Material* GetFontMaterial() const										{ return m_uiFontMaterial; }

	// XML helpers
	void ParseUIElementXml( const XmlElement& uiElementElem, UIElement* parentElem );
	UIAlignedPositionData ParseAlignedPositionData( const XmlElement& positionDataElem );
	//UIRelativePositionData ParseRelativePositionData( const XmlElement& positionDataElem );

private:
	RenderContext*	m_renderer = nullptr;
	InputSystem*	m_inputSystem = nullptr;
	Vec2			m_windowDimensions;

	UIPanel*		m_rootPanel = nullptr;

	BitmapFont*		m_uiFont = nullptr;
	Material*		m_uiFontMaterial = nullptr;

	std::map<std::string, uint> m_elementNameToId;
	std::map<uint, UIElement*> m_elementsById;
};
