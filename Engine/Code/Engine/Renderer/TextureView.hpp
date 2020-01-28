#pragma once
//-----------------------------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11RenderTargetView;
class Texture;


//-----------------------------------------------------------------------------------------------
class TextureView
{
public:
	TextureView();
	~TextureView();

public:
	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11RenderTargetView* m_renderTargetView;
	};
};
