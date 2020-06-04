#include "Game/UIButton.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/UIPanel.hpp"


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture )
	: m_boundingBox( absoluteScreenBounds )
	, m_backgroundTexture( backgroundTexture )
{

}


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture )
	: m_backgroundTexture( backgroundTexture )
{
	AABB2 boundingBox = parentPanel.GetBoundingBox();
	float width = boundingBox.GetWidth();
	float height = boundingBox.GetHeight();

	m_boundingBox.mins = Vec2( boundingBox.mins.x + relativeFractionMinPosition.x * width,
							   boundingBox.mins.y + relativeFractionMinPosition.y * height );

	m_boundingBox.maxs = Vec2( m_boundingBox.mins.x + relativeFractionOfDimensions.x * width,
							   m_boundingBox.mins.y + relativeFractionOfDimensions.y * height );
}


//-----------------------------------------------------------------------------------------------
UIButton::~UIButton()
{

}


//-----------------------------------------------------------------------------------------------
void UIButton::Update()
{
	if ( !m_isActive )
	{
		return;
	}

	if ( m_boundingBox.IsPointInside( g_inputSystem->GetNormalizedMouseClientPos() * Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ) )
	{
		if ( !m_isMouseHovering )
		{
			m_isMouseHovering = true;
			m_onHoverBeginEvent.Invoke( nullptr );
		}

		if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
		{
			m_onClickEvent.Invoke( nullptr );
		}
	}
	else if ( m_isMouseHovering )
	{
		m_isMouseHovering = false;
		m_onHoverEndEvent.Invoke( nullptr );
	}
}


//-----------------------------------------------------------------------------------------------
void UIButton::Render( RenderContext* renderer ) const
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
}


//-----------------------------------------------------------------------------------------------
void UIButton::Activate()
{
	m_isActive = true;
}


//-----------------------------------------------------------------------------------------------
void UIButton::Deactivate()
{
	m_isActive = false;
}


//-----------------------------------------------------------------------------------------------
void UIButton::Hide()
{
	m_isVisible = false;
}


//-----------------------------------------------------------------------------------------------
void UIButton::Show()
{
	m_isVisible = true;
}


//-----------------------------------------------------------------------------------------------
Vec2 UIButton::GetPosition()
{
	return m_boundingBox.GetCenter();
}
