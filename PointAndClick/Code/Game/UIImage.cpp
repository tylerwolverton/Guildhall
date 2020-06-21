#include "Game/UIImage.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image )
	: UILabel( parentElement, relativeFractionMinPosition, relativeFractionOfDimensions )
	, m_image( image )
{

}


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, SpriteDefinition* spriteDef )
	: UILabel( parentElement, relativeFractionMinPosition, relativeFractionOfDimensions )
{
	spriteDef->GetUVs( m_uvAtMins, m_uvAtMaxs );
	m_image = const_cast<Texture*>(&(spriteDef->GetTexture()));
}


//-----------------------------------------------------------------------------------------------
void UIImage::Render( RenderContext* renderer )
{
	if ( m_image != nullptr )
	{
		std::vector<Vertex_PCU> vertices;
		AppendVertsForAABB2D( vertices, m_boundingBox, m_tint, m_uvAtMins, m_uvAtMaxs );

		renderer->BindTexture( 0, m_image );
		renderer->DrawVertexArray( vertices );
	}
}
