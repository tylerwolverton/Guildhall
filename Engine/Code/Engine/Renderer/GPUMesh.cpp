#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"


//-----------------------------------------------------------------------------------------------
void GPUMesh::UpdateVertices( uint vertexCount, const void* vertexData, uint vertexStride, const BufferAttribute* layout )
{
	//m_vertices->Update(vertexData, )
}


//-----------------------------------------------------------------------------------------------
void GPUMesh::UpdateIndices( uint indexCount, const uint* indices )
{
	m_indices->Update( indexCount, indices );
}
