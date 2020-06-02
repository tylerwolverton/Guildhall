#include "Game/UIButton.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture )
	: m_boundingBox( absoluteScreenBounds )
	, m_backgroundTexture( backgroundTexture )
{

}


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const Vec2& dimensions, const Vec2& position, Texture* backgroundTexture )
	: m_boundingBox( AABB2( position, position + dimensions ) )
	, m_backgroundTexture( backgroundTexture )
{

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

		if ( g_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON ) )
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
		AppendVertsForAABB2D( vertices, m_boundingBox, Rgba8::WHITE );

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
