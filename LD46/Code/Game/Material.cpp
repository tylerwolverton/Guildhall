#include "Game/Material.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void Material::Bind()
{
	g_renderer->BindDiffuseTexture( m_diffuseTexture );
	g_renderer->BindNormalTexture( m_normalTexture );
	g_renderer->SetSampler( eSampler::POINT_WRAP );
	g_renderer->BindSampler( nullptr );

	g_renderer->BindShader( m_shader );

	//g_renderer->SetBlendMode( eBlendMode::ALPHA );
	//g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );
}


//-----------------------------------------------------------------------------------------------
//void Material::SetUserTexture( int slot, Texture* texture )
//{
//
//}
