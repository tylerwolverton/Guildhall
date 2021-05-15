#include "Engine/UI/UIPanel.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIUniformGrid.hpp"
#include "Engine/UI/UIImage.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIText.hpp"


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( UISystem& uiSystem, const AABB2& absoluteScreenBounds, Texture* backgroundTexture, const Rgba8& tint, const std::string& name )
	: UIElement( uiSystem, name )
{
	m_boundingBox = absoluteScreenBounds;
	m_initialBoundingBox = m_boundingBox;
	m_backgroundTexture = backgroundTexture;
	m_initialTint = tint;
	m_curTint = tint;
}


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( UISystem& uiSystem, UIPanel* parentPanel, const UIAlignedPositionData& positionData, 
				  Texture* backgroundTexture, const Rgba8& tint, 
				  const Vec2& uvAtMins, const Vec2& uvAtMaxs, 
				  const std::string& name )
	: UIElement( uiSystem, name )
{
	m_backgroundTexture = backgroundTexture;
	m_initialTint = tint;
	m_curTint = tint;
	m_uvsAtMins = uvAtMins;
	m_uvsAtMaxs = uvAtMaxs;

	AABB2 parentBoundingBox = AABB2( Vec2::ZERO, m_uiSystem.m_windowDimensions );
	if ( parentPanel != nullptr )
	{
		parentBoundingBox = parentPanel->GetBoundingBox();
	}

	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentBoundingBox, positionData );
	m_initialBoundingBox = m_boundingBox;
}


//-----------------------------------------------------------------------------------------------
UIPanel::UIPanel( UISystem& uiSystem, UIPanel* parentPanel, const UIRelativePositionData& positionData, 
				  Texture* backgroundTexture, const Rgba8& tint, 
				  const Vec2& uvAtMins, const Vec2& uvAtMaxs, 
				  const std::string& name )
	: UIElement( uiSystem, name )
{
	m_backgroundTexture = backgroundTexture;
	m_initialTint = tint;
	m_curTint = tint;
	m_uvsAtMins = uvAtMins;
	m_uvsAtMaxs = uvAtMaxs;

	AABB2 parentBoundingBox = AABB2( Vec2::ZERO, m_uiSystem.m_windowDimensions );

	if ( parentPanel != nullptr )
	{
		parentBoundingBox = parentPanel->GetBoundingBox();
	}

	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentBoundingBox, positionData );
	m_initialBoundingBox = m_boundingBox;
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

	// Consume clicks in panel to avoid world handling them
	if ( this != m_uiSystem.GetRootPanel()
		&& m_boundingBox.IsPointInside( m_uiSystem.m_inputSystem->GetNormalizedMouseClientPos() * m_uiSystem.m_windowDimensions ) )
	{
		//m_uiSystem.m_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON );
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
		AppendVertsForAABB2D( vertices, m_boundingBox, m_curTint, m_uvsAtMins, m_uvsAtMaxs );

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
UIPanel* UIPanel::AddChildPanel( const UIAlignedPositionData& positionData, Texture* backgroundTexture, const Rgba8& tint,
								 const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, positionData, backgroundTexture, tint, uvAtMins, uvAtMaxs );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const UIRelativePositionData& positionData, Texture* backgroundTexture, const Rgba8& tint,
								 const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, positionData, backgroundTexture, tint, uvAtMins, uvAtMaxs );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const std::string& name, const UIAlignedPositionData& positionData, 
								 Texture* backgroundTexture, const Rgba8& tint, 
								 const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, positionData, backgroundTexture, tint, uvAtMins, uvAtMaxs, name );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIPanel::AddChildPanel( const std::string& name, const UIRelativePositionData& positionData, 
								 Texture* backgroundTexture, const Rgba8& tint, 
								 const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UIPanel* newPanel = new UIPanel( m_uiSystem, this, positionData, backgroundTexture, tint, uvAtMins, uvAtMaxs, name );
	m_childPanels.push_back( newPanel );

	return newPanel;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const UIAlignedPositionData& positionData, 
							  Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, positionData, backgroundTexture, tint );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const UIRelativePositionData& positionData, 
							  Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, positionData, backgroundTexture, tint );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const std::string& name, const UIAlignedPositionData& positionData, 
							  Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, positionData, backgroundTexture, tint, name );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIPanel::AddButton( const std::string& name, const UIRelativePositionData& positionData, 
							  Texture* backgroundTexture, const Rgba8& tint )
{
	UIButton* newButton = new UIButton( m_uiSystem, *this, positionData, backgroundTexture, tint, name );
	m_buttons.push_back( newButton );

	return newButton;
}


//-----------------------------------------------------------------------------------------------
UIUniformGrid* UIPanel::AddUniformGrid( const std::string& name, const UIAlignedPositionData& gridPositionData, 
										const IntVec2& gridDimensions, const Vec2& paddingOfGridElementsPixels, 
										Texture* elementTexture, const Rgba8& elementTint )
{
	UIUniformGrid* newGrid = new UIUniformGrid( m_uiSystem, *this, name, gridPositionData, gridDimensions, paddingOfGridElementsPixels, elementTexture, elementTint );
	m_childPanels.push_back( newGrid );

	return newGrid;
}


//-----------------------------------------------------------------------------------------------
void UIPanel::ClearButtons()
{
	PTR_VECTOR_SAFE_DELETE( m_buttons );
}
