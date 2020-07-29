#include "Engine/UI/UILabel.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIElement.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const UISystem& uiSystem, const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions )
	: m_uiSystem( uiSystem )
{
	AABB2 boundingBox = parentElement.GetBoundingBox();
	float width = boundingBox.GetWidth();
	float height = boundingBox.GetHeight();

	m_boundingBox.mins = Vec2( boundingBox.mins.x + relativeFractionMinPosition.x * width,
							   boundingBox.mins.y + relativeFractionMinPosition.y * height );

	m_boundingBox.maxs = Vec2( m_boundingBox.mins.x + relativeFractionOfDimensions.x * width,
							   m_boundingBox.mins.y + relativeFractionOfDimensions.y * height );
}


//-----------------------------------------------------------------------------------------------
void UILabel::DebugRender() const
{
	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::GREEN, UI_DEBUG_LINE_THICKNESS );
}
