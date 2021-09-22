#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/D3D11/D3D11Common.hpp"


//-----------------------------------------------------------------------------------------------
TextureView::TextureView()
{
}


//-----------------------------------------------------------------------------------------------
TextureView::~TextureView()
{
	DX_SAFE_RELEASE( m_handle );
}
