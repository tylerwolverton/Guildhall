#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
struct ID3D11Buffer;
class RenderContext;


//-----------------------------------------------------------------------------------------------
// The "WHAT" are we using it for
enum eRenderBufferUsageBit : uint
{
	VERTEX_BUFFER_BIT	= BIT_FLAG( 0 ),	// A02: can be used to store vertices
	INDEX_BUFFER_BIT	= BIT_FLAG( 1 ),  	// we will discuss this later
	UNIFORM_BUFFER_BIT	= BIT_FLAG( 2 ),	// A03: used to store constants
};
typedef uint eRenderBufferUsage;			// typedef helps to describe to reader when an entire bit field can be sent as opposed to a single bit


//-----------------------------------------------------------------------------------------------
// The "HOW" are we going to access it
enum eRenderMemoryHint : uint
{
	MEMORY_HINT_GPU,		// GPU can read/write, CPU can't touch it. If we change it, it changes rarely from CPU
	MEMORY_HINT_DYNAMIC, 	// GPU memory (read/write), that changes OFTEN from CPU - it allows "Mapping" of memory 
	MEMORY_HINT_STAGING,	// CPU memory (read/write), can copy to a GPU buffer
};


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
