#include "Engine/UI/UIText.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UIText::UIText( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, const std::string& text, float fontSize, const Vec2& textAlignment )
	: UILabel( uiSystem, parentElement, positionData )
	, m_text( text )
	, m_fontSize( fontSize )
	, m_textAlignment( textAlignment )
{
	m_fontSize = ClampMinMax( m_fontSize, 0.f, m_boundingBox.GetHeight() );
}


//-----------------------------------------------------------------------------------------------
UIText::UIText( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, const std::string& text, float fontSize, const Vec2& textAlignment )
	: UILabel( uiSystem, parentElement, positionData )
	, m_text( text )
	, m_fontSize( fontSize )
	, m_textAlignment( textAlignment )
{
	m_fontSize = ClampMinMax( m_fontSize, 0.f, m_boundingBox.GetHeight() );
}


//-----------------------------------------------------------------------------------------------
void UIText::Render() const
{
	if ( !m_text.empty() )
	{
		std::vector<Vertex_PCU> vertices;
		BitmapFont* font = m_uiSystem.m_renderer->GetSystemFont();
		font->AppendVertsForTextInBox2D( vertices, m_boundingBox, m_fontSize, m_text, m_tint, 1.f, m_textAlignment );

		m_uiSystem.m_renderer->BindTexture( 0, font->GetTexture() );
		m_uiSystem.m_renderer->DrawVertexArray( vertices );
	}
}
