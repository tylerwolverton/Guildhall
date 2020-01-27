#pragma once

struct ID3D11Resource;
struct ID3D11ColorTargetView;
class Texture;


class TextureView
{
public:
	TextureView();
	~TextureView(); // just be sure to release the handle

public:
	Texture* m_owner = nullptr;

	union
	{
		ID3D11Resource* m_handle = nullptr;			// A01
		ID3D11ColorTargetView* m_rtv;				// A01 - alias as an rtv
	};
};
