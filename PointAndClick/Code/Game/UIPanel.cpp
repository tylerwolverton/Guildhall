#include "Game/UIPanel.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( const AABB2& absoluteScreenBounds, Texture* backgroundTexture )
	: m_boundingBox( absoluteScreenBounds )
	, m_backgroundTexture( backgroundTexture )
{
}


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( UIPanel* parentPanel, const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture )
	: m_backgroundTexture( backgroundTexture )
{
	m_boundingBox = AABB2( 0.f, 0.f, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS );

	if ( parentPanel != nullptr )
	{
		m_boundingBox = parentPanel->GetBoundingBox();
	}

	m_boundingBox.ChopOffLeft( widthFractionRange.x, 0.f );
	m_boundingBox.ChopOffRight( 1.f - widthFractionRange.y, 0.f );

	m_boundingBox.ChopOffBottom( heightFractionRange.x, 0.f );
	m_boundingBox.ChopOffTop( 1.f - heightFractionRange.y, 0.f );
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
void UIPanel::Activate()
{
	m_isActive = true;
}


//-----------------------------------------------------------------------------------------------
void UIPanel::Deactivate()
{
	m_isActive = false;
}


//-----------------------------------------------------------------------------------------------
void UIPanel::Hide()
{
	m_isVisible = false;
}


//-----------------------------------------------------------------------------------------------
void UIPanel::Show()
{
	m_isVisible = true;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture )
{
	UIPanel* newPanel = new UIPanel( this, widthFractionRange, heightFractionRange, backgroundTexture );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture )
{
	UIButton* newButton = new UIButton( *this, relativeFractionMinPosition, relativeFractionOfDimensions, backgroundTexture );
	m_buttons.push_back( newButton );

	return newButton;
}
