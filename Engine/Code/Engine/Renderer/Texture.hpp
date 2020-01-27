#pragma once
#include "Engine/Math/IntVec2.hpp"


//-----------------------------------------------------------------------------------------------
class TextureView;
class RenderContext;
struct ID3D11Texture2D;
struct ID3D11Resource;


//-----------------------------------------------------------------------------------------------
class Texture
{
public:
	Texture( RenderContext* owner, ID3D11Texture2D* handle ); // constructor we need for swapchain
	~Texture();
	
	TextureView* GetRenderTargetView();

public:
	RenderContext* m_renderContext = nullptr; // owning context

	// TODO - temp - for now we only have one view type, so we'll hard code to that
	// case, but in A03 we'll have multiple view types so we'll need to revisit this
	TextureView* m_renderTargetView = nullptr;

	// all texture types inherit from m_handle, 
	// and we'll be using `Texture` as basically an alias for all
	// texture types - more similar to more modern apis; 
	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11Texture2D* m_tex2D;
	};
};