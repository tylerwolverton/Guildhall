#include "Game/UILabel.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/UIElement.hpp"


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions )
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
void UILabel::DebugRender( RenderContext* renderer ) const
{
	renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( g_renderer, m_boundingBox, Rgba8::GREEN, UI_DEBUG_LINE_THICKNESS );
}
