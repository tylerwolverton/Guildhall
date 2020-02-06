#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


//-----------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( RenderContext* owner, eRenderMemoryHint memHint )
	: RenderBuffer( owner, VERTEX_BUFFER_BIT, memHint ) 
	, m_stride( sizeof( Vertex_PCU ))
	, m_attributes( Vertex_PCU::LAYOUT )
{

}

