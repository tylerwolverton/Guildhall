#include "Engine/Renderer/D3D11/D3D11RenderContext.hpp"
#include "Engine/Renderer/D3D11/D3D11Common.hpp"
#include "Engine/OS/Window.hpp"


//-----------------------------------------------------------------------------------------------
// DX3D11 Includes
//-----------------------------------------------------------------------------------------------
#pragma comment( lib, "d3d11.lib" )        
#pragma comment( lib, "dxgi.lib" )         
#pragma comment( lib, "d3dcompiler.lib" )   // needed when we get to shaders


//-----------------------------------------------------------------------------------------------
void D3D11RenderContext::Startup( Window* window )
{
	InitializeSwapChain( window );
	InitializeDefaultRenderObjects();

	RenderContext::Startup( window );
}


//-----------------------------------------------------------------------------------------------
void D3D11RenderContext::InitializeSwapChain( Window* window )
{
	IDXGISwapChain* swapchain = nullptr;

	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#if defined(RENDER_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	CreateDebugModule();
	#endif

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	memset( &swapchainDesc, 0, sizeof( swapchainDesc ) );

	// how many back buffers in our chain - we'll double buffer (one we show, one we draw to)
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // on swap, the old buffer is discarded
	swapchainDesc.Flags = 0; // additional flags - see docs.  Used in special cases like for video buffers

	// how swap chain is to be used
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	HWND hwnd = (HWND)window->m_hwnd;
	swapchainDesc.OutputWindow = hwnd;		// HWND for the window to be used
	swapchainDesc.SampleDesc.Count = 1;		// how many samples per pixel (1 so no MSAA)
											// note, if we're doing MSAA, we'll do it on a secondary target

	// describe the buffer
	swapchainDesc.Windowed = TRUE;                                    // windowed/full-screen mode
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color RGBA8 color
	swapchainDesc.BufferDesc.Width = window->GetClientWidth();
	swapchainDesc.BufferDesc.Height = window->GetClientHeight();

	HRESULT result = D3D11CreateDeviceAndSwapChain( nullptr,
													D3D_DRIVER_TYPE_HARDWARE,
													nullptr,
													flags, // controls the type of device we make
													nullptr,
													0,
													D3D11_SDK_VERSION,
													&swapchainDesc,
													&swapchain,
													&m_device,
													nullptr,
													&m_context );

	GUARANTEE_OR_DIE( SUCCEEDED( result ), "Failed to create rendering device." );

	if ( swapchain != nullptr )
	{
		//m_swapchain = new SwapChain( this, swapchain );
	}
}


//-----------------------------------------------------------------------------------------------
void D3D11RenderContext::InitializeDefaultRenderObjects()
{

}

