#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/D3DCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView.hpp"


//-----------------------------------------------------------------------------------------------
Texture::Texture( RenderContext* owner, ID3D11Texture2D* handle )
	: m_renderContext( owner )
	, m_handle( handle )
{
}


//-----------------------------------------------------------------------------------------------
Texture::~Texture()
{
}


//-----------------------------------------------------------------------------------------------
TextureView* Texture::GetRenderTargetView()
{ 
	// this uses lazy instantiation, meaning we'll only
	// make a view the first time it is requested.  As there
	// are many types of views, but most textures will at most use one or two.

	// only need to create it once
	if ( m_renderTargetView != nullptr )
	{
		return m_renderTargetView;
	}

	// now we create it.  If we need a special type of view
	// we would have to fill out a `D3D11_RENDER_TARGET_VIEW_DESC`, but 
	// in this case, the default will be fine; 

	ID3D11RenderTargetView* renderTargetView = nullptr;
	// get the device, since we're creating something
	m_renderContext->m_device->CreateRenderTargetView( m_handle, nullptr, &renderTargetView );

	if ( renderTargetView != nullptr )
	{
		// great, we made it, so make OUR object for it
		m_renderTargetView = new TextureView();
		m_renderTargetView->m_renderTargetView = renderTargetView; // setup the member
	}

	return m_renderTargetView;
}
