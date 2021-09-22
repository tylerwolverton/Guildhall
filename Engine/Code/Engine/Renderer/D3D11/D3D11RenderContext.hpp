#pragma once
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
class Window;


//-----------------------------------------------------------------------------------------------
class D3D11RenderContext : public RenderContext
{
public:
	virtual void Startup( Window* window ) override;

private:
	void InitializeSwapChain( Window* window );
	void InitializeDefaultRenderObjects();
};
