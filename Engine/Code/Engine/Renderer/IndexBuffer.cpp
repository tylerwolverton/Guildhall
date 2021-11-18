#include "Engine/Renderer/IndexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
IndexBuffer::IndexBuffer( RenderContext* owner, eRenderMemoryHint memHint )
	: RenderBuffer( owner, INDEX_BUFFER_BIT, memHint )
{

}


//-----------------------------------------------------------------------------------------------
void IndexBuffer::Update( uint indexCount, const uint* indices )
{
	size_t dataByteSize = indexCount * sizeof( uint );
	size_t elementSize = sizeof( uint );
	RenderBuffer::Update( indices, dataByteSize, elementSize );
}


//-----------------------------------------------------------------------------------------------
void IndexBuffer::Update( const std::vector<uint>& indices )
{
	Update( (uint)indices.size(), &indices[0] );
}
