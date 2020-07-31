#include "Engine/UI/UIButton.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UILabel.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const UISystem& uiSystem, const AABB2& absoluteScreenBounds, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_boundingBox = absoluteScreenBounds;
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;
}


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const UISystem& uiSystem, const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;

	AABB2 boundingBox = parentPanel.GetBoundingBox();
	float width = boundingBox.GetWidth();
	float height = boundingBox.GetHeight();

	m_boundingBox.mins = Vec2( boundingBox.mins.x + relativeFractionMinPosition.x * width,
							   boundingBox.mins.y + relativeFractionMinPosition.y * height );

	m_boundingBox.maxs = Vec2( m_boundingBox.mins.x + relativeFractionOfDimensions.x * width,
							   m_boundingBox.mins.y + relativeFractionOfDimensions.y * height );
}


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const UISystem& uiSystem, const UIPanel& parentPanel, const UIAlignedPositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;
	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentPanel.GetBoundingBox(), positionData );
}


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const UISystem& uiSystem, const UIPanel& parentPanel, const UIRelativePositionData& positionData, Texture* backgroundTexture, const Rgba8& tint )
	: UIElement( uiSystem )
{
	m_backgroundTexture = backgroundTexture;
	m_tint = tint;
	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentPanel.GetBoundingBox(), positionData );
}


//-----------------------------------------------------------------------------------------------
UIButton::~UIButton()
{
	PTR_VECTOR_SAFE_DELETE( m_labels );
}


//-----------------------------------------------------------------------------------------------
void UIButton::Update()
{
	if ( !m_isActive )
	{
		return;
	}

	if ( m_boundingBox.IsPointInside( m_uiSystem.m_inputSystem->GetNormalizedMouseClientPos() * m_uiSystem.m_windowDimensions ) )
	{
		if ( !m_isMouseHovering )
		{
			m_isMouseHovering = true;

			EventArgs args;
			args.SetValue( "id", m_id );
			args.SetValue( "button", (void*)this );
			m_onHoverBeginEvent.Invoke( &args );
		}
		else
		{
			EventArgs args;
			args.SetValue( "id", m_id );
			args.SetValue( "button", (void*)this );
			m_onHoverStayEvent.Invoke( &args );
		}

		if ( m_uiSystem.m_inputSystem->ConsumeAllKeyPresses( MOUSE_LBUTTON ) )
		{
			EventArgs args;
			args.SetValue( "id", m_id );
			args.SetValue( "button", (void*)this );
			m_onClickEvent.Invoke( &args );
		}
	}
	else if ( m_isMouseHovering )
	{
		m_isMouseHovering = false;

		EventArgs args;
		args.SetValue( "id", m_id );
		args.SetValue( "button", (void*)this );
		m_onHoverEndEvent.Invoke( &args );
	}
}


//-----------------------------------------------------------------------------------------------
void UIButton::Render() const
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
}


//-----------------------------------------------------------------------------------------------
void UIButton::DebugRender() const
{
	if ( !m_isVisible )
	{
		return;
	}

	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::CYAN, UI_DEBUG_LINE_THICKNESS );

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->DebugRender();
	}
}


//-----------------------------------------------------------------------------------------------
Vec2 UIButton::GetPosition() const
{
	return m_boundingBox.GetCenter();
}
