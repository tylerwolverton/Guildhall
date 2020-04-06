#include "Engine/Renderer/VertexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( RenderContext* owner, eRenderMemoryHint memHint, size_t stride, const BufferAttribute* attributes )
	: RenderBuffer( owner, VERTEX_BUFFER_BIT, memHint )
	, m_stride( stride )
	, m_attributes( attributes )
{

}