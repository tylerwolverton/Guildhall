#include "Engine/UI/UIPanel.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( const AABB2& absoluteScreenBounds, Texture* backgroundTexture )
	: m_boundingBox( absoluteScreenBounds )
	, m_backgroundTexture( backgroundTexture )
{
}


//-----------------------------------------------------------------------------------------------
UIPanel::~UIPanel()
{

}


//-----------------------------------------------------------------------------------------------
void UIPanel::Update()
{
	if ( !m_isActive )
	{
		return;
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
		AppendVertsForAABB2D( vertices, m_boundingBox, Rgba8::WHITE );

		renderer->BindTexture( 0, m_backgroundTexture );
		renderer->DrawVertexArray( vertices );
	}

	for ( int panelIdx = 0; panelIdx < (int)m_childPanels.size(); ++panelIdx )
	{
		m_childPanels[panelIdx].Render( renderer );
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
void UIPanel::CreateAndAddChildPanel( const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture )
{
	AABB2 childBoundingBox( m_boundingBox );

	childBoundingBox.ChopOffLeft( widthFractionRange.x, 0.f );
	childBoundingBox.ChopOffRight( 1.f - widthFractionRange.y, 0.f );

	childBoundingBox.ChopOffBottom( heightFractionRange.x, 0.f );
	childBoundingBox.ChopOffTop( 1.f - heightFractionRange.y, 0.f );

	UIPanel childPanel( childBoundingBox );
	childPanel.SetBackgroundTexture( backgroundTexture );

	m_childPanels.push_back( childPanel );
}
