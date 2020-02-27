#pragma once
#include "Engine/Renderer/D3D11Common.hpp"


//-----------------------------------------------------------------------------------------------
struct ID3D11Buffer;
class RenderContext;


//-----------------------------------------------------------------------------------------------
// A GPU Allocator (like malloc on the GPU, new byte[count])
class RenderBuffer
{
public:
	RenderBuffer( RenderContext* owner, eRenderBufferUsage usage, eRenderMemoryHint memHint );
	~RenderBuffer();

	bool Update( const void* data, size_t dataByteSize, size_t elementByteSize );

private:
	bool Create( size_t dataByteSize, size_t elementByteSize );

	bool IsCompatible( size_t dataByteSize, size_t elementByteSize ) const;
	void Cleanup();

public:
	RenderContext* m_owner = nullptr;
	ID3D11Buffer* m_handle = nullptr;
	eRenderBufferUsage m_usage;
	eRenderMemoryHint m_memHint;

	// stats
	size_t m_bufferByteSize = 0U;
	size_t m_elementByteSize = 0U; // used for vertex buffer  
};
