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
	~TextureView(); // just be sure to release the handle

public:
	// TODO in later assignment
	// add some identifying information about this type of view
	//Texture* m_owner = nullptr;

	ID3D11RenderTargetView* m_handle = nullptr;				// A01
};
