#pragma once
//-----------------------------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
class Texture;


//-----------------------------------------------------------------------------------------------
class TextureView
{
public:
	TextureView();
	~TextureView();

	/*ID3D11RenderTargetView* GetAsRTV() const { return m_renderTargetView; }
	ID3D11ShaderResourceView* GetAsSRV() const { return m_shaderResourceView; }*/

public:
	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11RenderTargetView* m_renderTargetView;
		ID3D11ShaderResourceView* m_shaderResourceView;
	};
};
