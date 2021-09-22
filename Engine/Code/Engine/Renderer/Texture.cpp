#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/D3D11/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView.hpp"


//-----------------------------------------------------------------------------------------------
Texture::Texture( RenderContext* owner, ID3D11Texture2D* handle )
	: m_owner( owner )
	, m_handle( handle )
{
	D3D11_TEXTURE2D_DESC desc;
	handle->GetDesc( &desc );

	m_texelSize = IntVec2( desc.Width, desc.Height );
}


//-----------------------------------------------------------------------------------------------
Texture::Texture( const char* filePath, RenderContext* owner, ID3D11Texture2D* handle )
	: Texture( owner, handle )
{
	m_filePath = filePath;
}


//-----------------------------------------------------------------------------------------------
Texture::~Texture()
{
	m_owner = nullptr;
	DX_SAFE_RELEASE( m_handle );

	PTR_SAFE_DELETE( m_renderTargetView );
	PTR_SAFE_DELETE( m_shaderResourceView );
	PTR_SAFE_DELETE( m_depthStencilView );
}


//-----------------------------------------------------------------------------------------------
TextureView* Texture::GetOrCreateRenderTargetView()
{ 
	// this uses lazy instantiation, meaning we'll only
	// make a view the first time it is requested.  As there
	// are many types of views, but most textures will at most use one or two.

	// only need to create it once
	if ( m_renderTargetView != nullptr )
	{
		return m_renderTargetView;
	}
	
	ID3D11RenderTargetView* renderTargetView = nullptr;
	// get the device, since we're creating something
	ID3D11Device* device = m_owner->m_device;
	device->CreateRenderTargetView( m_handle, nullptr, &renderTargetView );

	if ( renderTargetView != nullptr )
	{
		// great, we made it, so make OUR object for it
		m_renderTargetView = new TextureView(); // could pass in constructor, but would require a lot of constructors
		m_renderTargetView->m_renderTargetView = renderTargetView; // setup the member
	}

	return m_renderTargetView;
}


//-----------------------------------------------------------------------------------------------
TextureView* Texture::GetOrCreateShaderResourceView()
{
	if ( m_shaderResourceView != nullptr )
	{
		return m_shaderResourceView;
	}
	
	ID3D11Device* device = m_owner->m_device;
	ID3D11ShaderResourceView* shaderResourceView = nullptr;
	device->CreateShaderResourceView( m_handle, nullptr, &shaderResourceView );

	if ( shaderResourceView != nullptr )
	{
		// great, we made it, so make OUR object for it
		m_shaderResourceView = new TextureView(); 
		m_shaderResourceView->m_shaderResourceView = shaderResourceView; 
	}

	return m_shaderResourceView;
}


//-----------------------------------------------------------------------------------------------
TextureView* Texture::GetOrCreateDepthStencilView()
{
	if ( m_depthStencilView != nullptr )
	{
		return m_depthStencilView;
	}

	ID3D11Device* device = m_owner->m_device;
	ID3D11DepthStencilView* depthStencilView = nullptr;
	device->CreateDepthStencilView( m_handle, nullptr, &depthStencilView );

	if ( depthStencilView != nullptr )
	{
		// great, we made it, so make OUR object for it
		m_depthStencilView = new TextureView(); 
		m_depthStencilView->m_depthStencilView = depthStencilView;
	}

	return m_depthStencilView;
}


//-----------------------------------------------------------------------------------------------
float Texture::GetAspectRatio() const
{
	if ( m_texelSize.y == 0 )
	{
		return 0.f;
	}

	return (float)m_texelSize.x / (float)m_texelSize.y;
}
