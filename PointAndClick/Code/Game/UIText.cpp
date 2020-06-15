#include "Game/UIText.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
UIText::UIText( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, const std::string& text, float fontSize )
	: UILabel( parentButton, relativeFractionMinPosition, relativeFractionOfDimensions )
	, m_text( text )
	, m_fontSize( fontSize )
{

}


//-----------------------------------------------------------------------------------------------
void UIText::Render( RenderContext* renderer )
{
	if ( !m_text.empty() )
	{
		std::vector<Vertex_PCU> vertices;
		BitmapFont* font = renderer->GetSystemFont();
		font->AppendVertsForTextInBox2D( vertices, m_boundingBox, m_fontSize, m_text, m_tint );

		renderer->BindTexture( 0, font->GetTexture() );
		renderer->DrawVertexArray( vertices );
	}
}
