#include "Engine/UI/UISystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/UI/UIPanel.hpp"


//-----------------------------------------------------------------------------------------------
void UISystem::Startup( Window* window, RenderContext* renderer )
{
	m_renderer = renderer;
	m_inputSystem = window->GetInputSystem();
	m_windowDimensions = window->GetDimensions();

	m_rootPanel = new UIPanel( *this, AABB2( Vec2::ZERO, m_windowDimensions ) );
}


//-----------------------------------------------------------------------------------------------
void UISystem::Update()
{
	m_rootPanel->Update();
}


//-----------------------------------------------------------------------------------------------
void UISystem::Render()
{
	m_rootPanel->Render();
}


//-----------------------------------------------------------------------------------------------
void UISystem::DebugRender()
{
	m_rootPanel->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void UISystem::Shutdown()
{
	PTR_SAFE_DELETE( m_rootPanel );
}


//-----------------------------------------------------------------------------------------------
AABB2 UISystem::GetBoundingBoxFromParentAndPositionData( const AABB2& parentBoundingBox, const UIAlignedPositionData& positionData ) const
{
	float width = parentBoundingBox.GetWidth() * positionData.fractionOfParentDimensions.x;
	float height = parentBoundingBox.GetHeight() * positionData.fractionOfParentDimensions.y;

	AABB2 boundingBox = parentBoundingBox;
	boundingBox.mins = parentBoundingBox.mins;
	boundingBox.maxs = parentBoundingBox.mins + Vec2( width, height );

	boundingBox.AlignWithinBounds( parentBoundingBox, positionData.alignmentWithinParentElement );

	PadBoundingBox( boundingBox, positionData.horizontalPaddingInPixels, positionData.verticalPaddingInPixels );
	OffsetBoundingBox( boundingBox, parentBoundingBox, positionData.positionOffsetFraction, positionData.positionOffsetInPixels );

	return boundingBox;
}


//-----------------------------------------------------------------------------------------------
AABB2 UISystem::GetBoundingBoxFromParentAndPositionData( const AABB2& parentBoundingBox, const UIRelativePositionData& positionData ) const
{
	// Set position based on parent and specified dimension fractions
	AABB2 boundingBox = parentBoundingBox;
	float width = parentBoundingBox.GetWidth();
	float height = parentBoundingBox.GetHeight();

	boundingBox.mins = Vec2( boundingBox.mins.x + positionData.widthFractionRange.x * width,
							 boundingBox.mins.y + positionData.heightFractionRange.x * height );

	boundingBox.maxs = Vec2( boundingBox.mins.x + positionData.widthFractionRange.y * width,
							 boundingBox.mins.y + positionData.heightFractionRange.y * height );

	PadBoundingBox( boundingBox, positionData.horizontalPaddingInPixels, positionData.verticalPaddingInPixels );
	OffsetBoundingBox( boundingBox, parentBoundingBox, positionData.positionOffsetFraction, positionData.positionOffsetInPixels );

	return boundingBox;
}


//-----------------------------------------------------------------------------------------------
void UISystem::PadBoundingBox( AABB2& boundingBox, const Vec2& horizontalPaddingInPixels, const Vec2& verticalPaddingInPixels ) const
{
	boundingBox.mins.x += horizontalPaddingInPixels.x;
	boundingBox.maxs.x -= horizontalPaddingInPixels.y;

	boundingBox.mins.y += verticalPaddingInPixels.x;
	boundingBox.maxs.y -= verticalPaddingInPixels.y;
}


//-----------------------------------------------------------------------------------------------
void UISystem::OffsetBoundingBox( AABB2& boundingBox, const AABB2& parentBoundingBox, const Vec2& offsetFraction, const Vec2& offsetInPixels ) const
{
	Vec2 relativeOffset = offsetFraction * Vec2( parentBoundingBox.GetWidth(), parentBoundingBox.GetHeight() );
	boundingBox.mins += relativeOffset;
	boundingBox.maxs += relativeOffset;

	boundingBox.mins += offsetInPixels;
	boundingBox.maxs += offsetInPixels;
}
