#include "Engine/UI/UIPanel.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIImage.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIText.hpp"


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( const UISystem& uiSystem, const AABB2& absoluteScreenBounds, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_boundingBox = absoluteScreenBounds;
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;
}


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( const UISystem& uiSystem, UIPanel* parentPanel, const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;
	
	m_boundingBox = AABB2( Vec2::ZERO, m_uiSystem.m_windowDimensions );
	
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
UIPanel::UIPanel( const UISystem& uiSystem, UIPanel* parentPanel, const UIAlignedPositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;

	AABB2 parentBoundingBox = AABB2( Vec2::ZERO, m_uiSystem.m_windowDimensions );
	if ( parentPanel != nullptr )
	{
		parentBoundingBox = parentPanel->GetBoundingBox();
	}

	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentBoundingBox, positionData );
}


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( const UISystem& uiSystem, UIPanel* parentPanel, const UIRelativePositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;

	AABB2 parentBoundingBox = AABB2( Vec2::ZERO, m_uiSystem.m_windowDimensions );

	if ( parentPanel != nullptr )
	{
		parentBoundingBox = parentPanel->GetBoundingBox();
	}

	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentBoundingBox, positionData );
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
void UIPanel::Render() const
{
	if ( !m_isVisible )
	{
		return;
	}

	if ( m_backgroundTexture != nullptr )
	{
		std::vector<Vertex_PCU> vertices;
		AppendVertsForAABB2D( vertices, m_boundingBox, m_tint );

		m_uiSystem.m_renderer->BindTexture( 0, m_backgroundTexture );
		m_uiSystem.m_renderer->DrawVertexArray( vertices );
	}

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->Render();
	}

	for ( int buttonIdx = 0; buttonIdx < (int)m_buttons.size(); ++buttonIdx )
	{
		m_buttons[buttonIdx]->Render();
	}

	for ( int panelIdx = 0; panelIdx < (int)m_childPanels.size(); ++panelIdx )
	{
		m_childPanels[panelIdx]->Render();
	}
}


//-----------------------------------------------------------------------------------------------
void UIPanel::DebugRender() const
{
	if ( !m_isVisible )
	{
		return;
	}

	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::MAGENTA, UI_DEBUG_LINE_THICKNESS );

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->DebugRender();
	}

	for ( int buttonIdx = 0; buttonIdx < (int)m_buttons.size(); ++buttonIdx )
	{
		m_buttons[buttonIdx]->DebugRender();
	}

	for ( int panelIdx = 0; panelIdx < (int)m_childPanels.size(); ++panelIdx )
	{
		m_childPanels[panelIdx]->DebugRender();
	}
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture, const Rgba8& tint )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, widthFractionRange, heightFractionRange, backgroundTexture, tint );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const UIAlignedPositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, positionData, backgroundTexture, tint );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const UIRelativePositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, positionData, backgroundTexture, tint );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, relativeFractionMinPosition, relativeFractionOfDimensions, backgroundTexture, tint );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const UIAlignedPositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, positionData, backgroundTexture, tint );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const UIRelativePositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, positionData, backgroundTexture, tint );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
void UIPanel::ClearButtons()
{
	PTR_VECTOR_SAFE_DELETE( m_buttons );
}
