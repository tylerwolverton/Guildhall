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
UILabel::UILabel( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData )
	: m_uiSystem( uiSystem )
{
	// Set size of box based on parent
	AABB2 boundingBox = parentElement.GetBoundingBox();
	float width = boundingBox.GetWidth() * positionData.fractionOfParentDimensions.x;
	float height = boundingBox.GetHeight() * positionData.fractionOfParentDimensions.y;

	m_boundingBox.mins = boundingBox.mins;
	m_boundingBox.maxs = boundingBox.mins + Vec2( width, height );

	m_boundingBox.AlignWithinBounds( boundingBox, positionData.alignmentWithinParentElement );

	// Pad sides of box inward
	m_boundingBox.mins.x += positionData.horizontalPaddingInPixels.x;
	m_boundingBox.maxs.x -= positionData.horizontalPaddingInPixels.y;

	m_boundingBox.mins.y += positionData.verticalPaddingInPixels.x;
	m_boundingBox.maxs.y -= positionData.verticalPaddingInPixels.y;

	// Move box by offset
	m_boundingBox.mins += positionData.positionOffsetInPixels;
	m_boundingBox.maxs += positionData.positionOffsetInPixels;
}


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData )
	: m_uiSystem( uiSystem )
{
	// Set position based on parent and specified dimension fractions
	AABB2 boundingBox = parentElement.GetBoundingBox();
	float width = boundingBox.GetWidth();
	float height = boundingBox.GetHeight();

	m_boundingBox.mins = Vec2( boundingBox.mins.x + positionData.widthFractionRange.x * width,
							   boundingBox.mins.y + positionData.heightFractionRange.x * height );

	m_boundingBox.maxs = Vec2( m_boundingBox.mins.x + positionData.widthFractionRange.y * width,
							   m_boundingBox.mins.y + positionData.heightFractionRange.y * height );

	// Pad sides of box inward
	m_boundingBox.mins.x += positionData.horizontalPaddingInPixels.x;
	m_boundingBox.maxs.x -= positionData.horizontalPaddingInPixels.y;

	m_boundingBox.mins.y += positionData.verticalPaddingInPixels.x;
	m_boundingBox.maxs.y -= positionData.verticalPaddingInPixels.y;

	// Move box by offset
	m_boundingBox.mins += positionData.positionOffsetInPixels;
	m_boundingBox.maxs += positionData.positionOffsetInPixels;
}


//-----------------------------------------------------------------------------------------------
void UILabel::DebugRender() const
{
	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::GREEN, UI_DEBUG_LINE_THICKNESS );
}
