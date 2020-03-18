#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
SwapChain::SwapChain( RenderContext* owner, IDXGISwapChain* handle )
	: m_owner( owner )
	, m_handle( handle )
{
}


//-----------------------------------------------------------------------------------------------
SwapChain::~SwapChain()
{
	m_owner = nullptr;
	DX_SAFE_RELEASE( m_handle );

	PTR_SAFE_DELETE( m_backbuffer );
}


//-----------------------------------------------------------------------------------------------
Texture* SwapChain::GetBackBuffer()
{
	if ( m_backbuffer != nullptr )
	{
		return m_backbuffer;
	}

	// first, we request the D3D11 handle of the textures owned by the swapbuffer
	ID3D11Texture2D* texHandle = nullptr;
	m_handle->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)& texHandle );

	GUARANTEE_OR_DIE( texHandle != nullptr, "Failed to get textures from swapchain buffer." );

	// once we have the handle, we'll wrap it in our class to make the interface easy to use
	// we are giving our reference we got from `GetBuffer` to this texture, who will handle
	// it from here on; 
	m_backbuffer = new Texture( m_owner, texHandle );

	return m_backbuffer;
}


//-----------------------------------------------------------------------------------------------
void SwapChain::Present( int vsync )
{
	m_handle->Present( vsync,	// vsync - 0 if you don't want to wait
					   0 );		// DXGI_PRESENT option, see docs
}
