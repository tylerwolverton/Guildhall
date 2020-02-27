#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
Sampler::Sampler( RenderContext* context, eSamplerType type )
	: m_owner( context )
{
	ID3D11Device* device = context->m_device;

	D3D11_SAMPLER_DESC desc;
	if ( type == SAMPLER_POINT )
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	else if ( type == SAMPLER_BILINEAR )
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	}
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MipLODBias = 0;
	desc.MaxAnisotropy = 0;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.BorderColor[0] = 0.f;
	desc.BorderColor[1] = 0.f;
	desc.BorderColor[2] = 0.f;
	desc.BorderColor[3] = 0.f;
	desc.MinLOD = 0.f;
	desc.MaxLOD = 0.f;

	device->CreateSamplerState( &desc, &m_handle );
}


//-----------------------------------------------------------------------------------------------
Sampler::~Sampler()
{
	DX_SAFE_RELEASE( m_handle );
}
