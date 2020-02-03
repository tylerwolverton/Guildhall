#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"


//-----------------------------------------------------------------------------------------------
class VertexBuffer : public RenderBuffer
{
public:
	VertexBuffer( RenderContext* owner, eRenderMemoryHint memHint );

public:
	// information about our vertex
	size_t m_stride = 0; // how far from one vertex to the next
	//const buffer_attribute_t* m_attributes; // array describing the vertex
};