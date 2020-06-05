#include "Game/UIImage.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image )
	: UILabel( parentButton, relativeFractionMinPosition, relativeFractionOfDimensions )
	, m_image( image )
{

}


//-----------------------------------------------------------------------------------------------
void UIImage::Render( RenderContext* renderer )
{
	if ( m_image != nullptr )
	{
		std::vector<Vertex_PCU> vertices;
		AppendVertsForAABB2D( vertices, m_boundingBox, m_tint );

		renderer->BindTexture( 0, m_image );
		renderer->DrawVertexArray( vertices );
	}
}
