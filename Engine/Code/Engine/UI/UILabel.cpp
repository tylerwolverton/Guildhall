#include "Engine/UI/UILabel.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIElement.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData )
	: m_uiSystem( uiSystem )
{
	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentElement.GetBoundingBox(), positionData );
}


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData )
	: m_uiSystem( uiSystem )
{
	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentElement.GetBoundingBox(), positionData );
}


//-----------------------------------------------------------------------------------------------
void UILabel::DebugRender() const
{
	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::GREEN, UI_DEBUG_LINE_THICKNESS );
}
