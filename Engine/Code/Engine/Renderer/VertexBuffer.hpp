#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"


//-----------------------------------------------------------------------------------------------
struct BufferAttribute;


//-----------------------------------------------------------------------------------------------
class VertexBuffer : public RenderBuffer
{
public:
	VertexBuffer( RenderContext* owner, eRenderMemoryHint memHint, size_t stride, const BufferAttribute* attributes );

public:
	// information about our vertex
	size_t m_stride = 0; // how far from one vertex to the next
	const BufferAttribute* m_attributes = nullptr; // array describing the vertex
};