#include "Engine/Renderer/VertexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( RenderContext* owner, eRenderMemoryHint memHint )
	: RenderBuffer( owner, VERTEX_BUFFER_BIT, memHint ) 
{

}

