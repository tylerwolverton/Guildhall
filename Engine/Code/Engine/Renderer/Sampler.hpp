#pragma once
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
struct ID3D11SamplerState;
class RenderContext;


//-----------------------------------------------------------------------------------------------
enum eSamplerType : uint
{
	SAMPLER_POINT,		// pixelated look (nearest)
	SAMPLER_BILINEAR	// smoother look - no mips
};


//-----------------------------------------------------------------------------------------------
enum eSamplerUVMode : uint
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

	eSamplerType m_filter = SAMPLER_POINT;
	eSamplerUVMode m_mode = UV_MODE_CLAMP;
};