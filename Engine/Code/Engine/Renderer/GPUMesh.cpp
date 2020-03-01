#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
GPUMesh::GPUMesh( RenderContext* context )
{
	m_vertices = new VertexBuffer( context, MEMORY_HINT_DYNAMIC );
	m_indices = new IndexBuffer( context, MEMORY_HINT_DYNAMIC );
}

//-----------------------------------------------------------------------------------------------
GPUMesh::~GPUMesh()
{
	delete m_vertices;
	delete m_indices;
}


//-----------------------------------------------------------------------------------------------
void GPUMesh::UpdateVertices( uint vertexCount, const void* vertexData, uint vertexStride, const BufferAttribute* layout )
{
	UNUSED( layout );

	size_t dataByteSize = (size_t)vertexCount * (size_t)vertexStride;
	size_t elementSize = vertexStride;
	m_vertices->Update( vertexData, dataByteSize, elementSize );
}


//-----------------------------------------------------------------------------------------------
int GPUMesh::GetIndexCount()
{
	return (int)m_indices->m_bufferByteSize / (int)m_indices->m_elementByteSize;
}


//-----------------------------------------------------------------------------------------------
int GPUMesh::GetVertexCount()
{
	return (int)m_vertices->m_bufferByteSize / (int)m_vertices->m_elementByteSize;
}


//-----------------------------------------------------------------------------------------------
void GPUMesh::UpdateIndices( uint indexCount, const uint* indices )
{
	m_indices->Update( indexCount, indices );
}


//-----------------------------------------------------------------------------------------------
void GPUMesh::UpdateIndices( const std::vector<uint>& indices )
{
	UpdateIndices( (uint)indices.size(), &indices[0] );
}
