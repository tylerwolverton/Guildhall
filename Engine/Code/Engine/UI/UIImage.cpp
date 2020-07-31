#include "Engine/UI/UIImage.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, Texture* image )
	: UILabel( uiSystem, parentElement, positionData )
	, m_image( image )
{
}


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, Texture* image )
	: UILabel( uiSystem, parentElement, positionData )
	, m_image( image )
{
}


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef )
	: UILabel( uiSystem, parentElement, positionData )
{
	if ( spriteDef == nullptr )
	{
		return;
	}

	spriteDef->GetUVs( m_uvAtMins, m_uvAtMaxs );
	m_image = const_cast<Texture*>( &( spriteDef->GetTexture() ) );
}


//-----------------------------------------------------------------------------------------------
UIImage::UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, SpriteDefinition* spriteDef )
	: UILabel( uiSystem, parentElement, positionData )
{
	if ( spriteDef == nullptr )
	{
		return;
	}

	spriteDef->GetUVs( m_uvAtMins, m_uvAtMaxs );
	m_image = const_cast<Texture*>( &( spriteDef->GetTexture() ) );
}


//-----------------------------------------------------------------------------------------------
void UIImage::Render() const
{
	if ( m_image != nullptr )
	{
		std::vector<Vertex_PCU> vertices;
		AppendVertsForAABB2D( vertices, m_boundingBox, m_tint, m_uvAtMins, m_uvAtMaxs );

		m_uiSystem.m_renderer->BindTexture( 0, m_image );
		m_uiSystem.m_renderer->DrawVertexArray( vertices );
	}
}
