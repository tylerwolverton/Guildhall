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
	size_t dataByteSize = vertexCount * sizeof( Vertex_PCU );
	size_t elementSize = sizeof( Vertex_PCU );
	m_vertices->Update( vertexData, dataByteSize, elementSize );
	//m_vertices->Update(vertexData, )
}


//-----------------------------------------------------------------------------------------------
void GPUMesh::UpdateIndices( uint indexCount, const uint* indices )
{
	m_indices->Update( indexCount, indices );
}
