#pragma once

//-----------------------------------------------------------------------------------------------
class Texture;
class RenderContext;
struct IDXGISwapChain;


//-----------------------------------------------------------------------------------------------
class SwapChain
{
public:
	SwapChain( RenderContext* owner, IDXGISwapChain* handle );
	~SwapChain();

	Texture* GetBackBuffer();	// what do we want to render to?
	void Present( int vsync = 0 );				// does the job of `SwapBuffers`

public:
	RenderContext* m_owner = nullptr; // owning context
	IDXGISwapChain* m_handle = nullptr;

	Texture* m_backbuffer = nullptr;
};
