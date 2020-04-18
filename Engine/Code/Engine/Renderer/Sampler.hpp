#pragma once
#include "Engine/Core/Rgba8.hpp"


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
enum eSamplerUVMode
{
	UV_MODE_CLAMP,		
	UV_MODE_WRAP
};


//-----------------------------------------------------------------------------------------------
class Sampler
{
public:
	Sampler( RenderContext* context, eSamplerType type, eSamplerUVMode mode );
	~Sampler();

	//inline ID3D11SamplerState* GetHandle() const							{ return m_handle; }

public:
	RenderContext* m_owner = nullptr;
	ID3D11SamplerState* m_handle = nullptr;
};