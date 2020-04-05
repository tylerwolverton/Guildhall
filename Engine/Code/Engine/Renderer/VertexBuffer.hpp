#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"


//-----------------------------------------------------------------------------------------------
struct BufferAttribute;


//-----------------------------------------------------------------------------------------------
class VertexBuffer : public RenderBuffer
{
public:
	template <typename VERTEX_TYPE>
	VertexBuffer( RenderContext* owner, eRenderMemoryHint memHint, VERTEX_TYPE vertex )
		: RenderBuffer( owner, VERTEX_BUFFER_BIT, memHint )
		, m_stride( sizeof( VERTEX_TYPE ) )
		, m_attributes( VERTEX_TYPE::LAYOUT )
	{

	}

public:
	// information about our vertex
	size_t m_stride = 0; // how far from one vertex to the next
	const BufferAttribute* m_attributes = nullptr; // array describing the vertex
};