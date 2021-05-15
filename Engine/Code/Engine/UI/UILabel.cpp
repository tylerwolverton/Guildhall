#include "Engine/UI/UILabel.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIElement.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const std::string& name, UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData )
	: UIElement( uiSystem, name )
{
	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentElement.GetBoundingBox(), positionData );
}


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const std::string& name, UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData )
	: UIElement( uiSystem, name )
{
	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentElement.GetBoundingBox(), positionData );
}


//-----------------------------------------------------------------------------------------------
void UILabel::DebugRender() const
{
	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::GREEN, UI_DEBUG_LINE_THICKNESS );
}
