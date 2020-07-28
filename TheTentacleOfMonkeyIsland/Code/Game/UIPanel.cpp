#include "Game/UIPanel.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/UIButton.hpp"
#include "Game/UIText.hpp"
#include "Game/UIImage.hpp"


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( const AABB2& absoluteScreenBounds, Texture* backgroundTexture, const Rgba8& tint )
{
	m_boundingBox = absoluteScreenBounds;
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;
}


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( UIPanel* parentPanel, const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture, const Rgba8& tint )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;

	m_boundingBox = AABB2( 0.f, 0.f, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS );

	if ( parentPanel != nullptr )
	{
		m_boundingBox = parentPanel->GetBoundingBox();
	}

	AABB2 leftBox = m_boundingBox.GetBoxAtLeft( widthFractionRange.x, 0.f );
	AABB2 rightBox = m_boundingBox.GetBoxAtRight( 1.f - widthFractionRange.y, 0.f );

	AABB2 bottomBox = m_boundingBox.GetBoxAtBottom( heightFractionRange.x, 0.f );
	AABB2 topBox = m_boundingBox.GetBoxAtTop( 1.f - heightFractionRange.y, 0.f );

	m_boundingBox.mins.x = leftBox.maxs.x;
	m_boundingBox.maxs.x = rightBox.mins.x;

	m_boundingBox.mins.y = bottomBox.maxs.y;
	m_boundingBox.maxs.y = topBox.mins.y;
}


//-----------------------------------------------------------------------------------------------
UIPanel::~UIPanel()
{
	PTR_VECTOR_SAFE_DELETE( m_buttons );
	PTR_VECTOR_SAFE_DELETE( m_childPanels );
}


//-----------------------------------------------------------------------------------------------
void UIPanel::Update()
{
	if ( !m_isActive )
	{
		return;
	}

	for ( int buttonIdx = 0; buttonIdx < (int)m_buttons.size(); ++buttonIdx )
	{
		m_buttons[buttonIdx]->Update();
	}

	for ( int panelIdx = 0; panelIdx < (int)m_childPanels.size(); ++panelIdx )
	{
		m_childPanels[panelIdx]->Update();
	}
}


//-----------------------------------------------------------------------------------------------
void UIPanel::Render( RenderContext* renderer ) const
{
	if ( !m_isVisible )
	{
		return;
	}

	if ( m_backgroundTexture != nullptr )
	{
		std::vector<Vertex_PCU> vertices;
		AppendVertsForAABB2D( vertices, m_boundingBox, m_tint );

		renderer->BindTexture( 0, m_backgroundTexture );
		renderer->DrawVertexArray( vertices );
	}

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->Render( renderer );
	}

	for ( int buttonIdx = 0; buttonIdx < (int)m_buttons.size(); ++buttonIdx )
	{
		m_buttons[buttonIdx]->Render( renderer );
	}

	for ( int panelIdx = 0; panelIdx < (int)m_childPanels.size(); ++panelIdx )
	{
		m_childPanels[panelIdx]->Render( renderer );
	}
}


//-----------------------------------------------------------------------------------------------
void UIPanel::DebugRender( RenderContext* renderer ) const
{
	if ( !m_isVisible )
	{
		return;
	}

	renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( g_renderer, m_boundingBox, Rgba8::MAGENTA, UI_DEBUG_LINE_THICKNESS );

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->DebugRender( renderer );
	}

	for ( int buttonIdx = 0; buttonIdx < (int)m_buttons.size(); ++buttonIdx )
	{
		m_buttons[buttonIdx]->DebugRender( renderer );
	}

	for ( int panelIdx = 0; panelIdx < (int)m_childPanels.size(); ++panelIdx )
	{
		m_childPanels[panelIdx]->DebugRender( renderer );
	}
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture, const Rgba8& tint )
{
	UIPanel* newPanel = new UIPanel( this, widthFractionRange, heightFractionRange, backgroundTexture, tint );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( *this, relativeFractionMinPosition, relativeFractionOfDimensions, backgroundTexture, tint );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
void UIPanel::ClearButtons()
{
	PTR_VECTOR_SAFE_DELETE( m_buttons );
}
