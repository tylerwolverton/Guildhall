#pragma once
#include "Engine/Math/IntVec2.hpp"

#include <string>


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
	Texture( const char* filePath, RenderContext* owner, ID3D11Texture2D* handle ); 
	~Texture();
	
	TextureView* GetOrCreateRenderTargetView();
	TextureView* GetOrCreateShaderResourceView();
	TextureView* GetOrCreateDepthStencilView();

	IntVec2	GetTexelSize()												{ return m_texelSize; }
	std::string& GetFilePath()											{ return m_filePath; }

	// Could change to having static creator in class
	//static Texture* CreateFromFile( RenderContext* context, const char* filename );

public:
	RenderContext* m_owner = nullptr; // owning context

	
	TextureView* m_renderTargetView = nullptr;
	TextureView* m_shaderResourceView = nullptr;
	TextureView* m_depthStencilView = nullptr;

	// all texture types inherit from m_handle, 
	// and we'll be using `Texture` as basically an alias for all
	// texture types - more similar to more modern apis; 
	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11Texture2D* m_tex2D;
	};

private:
	std::string		m_filePath;
	IntVec2			m_texelSize = IntVec2::ZERO;
};
