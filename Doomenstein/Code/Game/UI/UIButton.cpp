#include "Game/UI/UIButton.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/UI/UIPanel.hpp"
#include "Game/UI/UILabel.hpp"
#include "Game/UI/UIImage.hpp"
#include "Game/UI/UIText.hpp"


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture, const Rgba8& tint )
	: m_boundingBox( absoluteScreenBounds )
	, m_backgroundTexture( backgroundTexture )
	, m_tint( tint )
{
	m_id = UIPanel::GetNextId();
}


//-----------------------------------------------------------------------------------------------
UIButton::UIButton( const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture, const Rgba8& tint )
	: m_backgroundTexture( backgroundTexture )
	, m_tint( tint )
{
	m_id = UIPanel::GetNextId();

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
	PTR_VECTOR_SAFE_DELETE( m_labels );
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

			EventArgs args;
			args.SetValue( "id", m_id );
			m_onHoverBeginEvent.Invoke( &args );
		}
		else
		{
			EventArgs args;
			args.SetValue( "id", m_id );
			m_onHoverStayEvent.Invoke( &args );
		}

		if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
		{
			EventArgs args;
			args.SetValue( "id", m_id );

			m_onClickEvent.Invoke( &args );
		}
	}
	else if ( m_isMouseHovering )
	{
		m_isMouseHovering = false;

		EventArgs args;
		args.SetValue( "id", m_id );
		m_onHoverEndEvent.Invoke( &args );
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

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->Render( renderer );
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
Vec2 UIButton::GetPosition() const
{
	return m_boundingBox.GetCenter();
}


//-----------------------------------------------------------------------------------------------
UILabel* UIButton::AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image )
{
	UILabel* newImage = new UIImage( *this, relativeFractionMinPosition, relativeFractionOfDimensions, image );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIButton::AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, SpriteDefinition* spriteDef )
{
	UILabel* newImage = new UIImage( *this, relativeFractionMinPosition, relativeFractionOfDimensions, spriteDef );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIButton::AddText( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, const std::string& text )
{
	UILabel* newText = new UIText( *this, relativeFractionMinPosition, relativeFractionOfDimensions, text );
	m_labels.push_back( newText );

	return newText;
}


//-----------------------------------------------------------------------------------------------
void UIButton::ClearLabels()
{
	PTR_VECTOR_SAFE_DELETE( m_labels );
}


