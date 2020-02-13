#pragma once
//-----------------------------------------------------------------------------------------------
struct ID3D11SamplerState;
class RenderContext;


//-----------------------------------------------------------------------------------------------
enum eSamplerType
{
	SAMPLER_POINT,		// pixelated look (nearest)
	SAMPLER_BILINEAR	// smoother look - no mips
};


//-----------------------------------------------------------------------------------------------
class Sampler
{
public:
	Sampler( RenderContext* context, eSamplerType type );
	~Sampler();

	//inline ID3D11SamplerState* GetHandle() const							{ return m_handle; }

public:
	RenderContext* m_owner = nullptr;
	ID3D11SamplerState* m_handle = nullptr;
};