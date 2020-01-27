#pragma once
//-----------------------------------------------------------------------------------------------
class Texture;
class RenderContext;
struct IDXGISwapChain;


//-----------------------------------------------------------------------------------------------
class SwapChain
{
public:
	SwapChain( RenderContext* renderContext, IDXGISwapChain* swapchain );
	~SwapChain();

	Texture* GetBackBuffer();	// what do we want to render to?
	void Present();				// does the job of `SwapBuffers`


public:
	RenderContext* m_renderContext = nullptr; // owning context
	IDXGISwapChain* m_swapchain = nullptr;

	Texture* m_backbuffer = nullptr;
};
