#include "Engine/UI/UIText.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UIText::UIText( const UISystem& uiSystem, const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, const std::string& text, float fontSize, const Vec2& alignment )
	: UILabel( uiSystem, parentElement, relativeFractionMinPosition, relativeFractionOfDimensions )
	, m_text( text )
	, m_fontSize( fontSize )
	, m_alignment( alignment )
{

}


//-----------------------------------------------------------------------------------------------
void UIText::Render() const
{
	if ( !m_text.empty() )
	{
		std::vector<Vertex_PCU> vertices;
		BitmapFont* font = m_uiSystem.m_renderer->GetSystemFont();
		font->AppendVertsForTextInBox2D( vertices, m_boundingBox, m_fontSize, m_text, m_tint, 1.f, m_alignment );

		m_uiSystem.m_renderer->BindTexture( 0, font->GetTexture() );
		m_uiSystem.m_renderer->DrawVertexArray( vertices );
	}
}
