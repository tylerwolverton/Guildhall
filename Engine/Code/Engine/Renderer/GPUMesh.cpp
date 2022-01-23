#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
GPUMesh::~GPUMesh()
{
	PTR_SAFE_DELETE( m_vertices );
	PTR_SAFE_DELETE( m_indices );
}


//-----------------------------------------------------------------------------------------------
GPUMesh::GPUMesh( RenderContext* context, const CPUMesh& cpuMesh )
{
	m_context = context;
	SetFromCPUMesh( cpuMesh );
}


//-----------------------------------------------------------------------------------------------
void GPUMesh::SetFromCPUMesh( const CPUMesh& cpuMesh )
{
	PTR_SAFE_DELETE( m_vertices );
	PTR_SAFE_DELETE( m_indices );

	m_vertices = new VertexBuffer( m_context, MEMORY_HINT_DYNAMIC, sizeof( Vertex_PCUTBN ), Vertex_PCUTBN::LAYOUT );
	m_indices = new IndexBuffer( m_context, MEMORY_HINT_DYNAMIC );

	UpdateVertices( cpuMesh.GetVertices() );
	UpdateIndices( cpuMesh.GetIndices() );
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
	if ( (int)m_indices->m_elementByteSize == 0 )
	{
		return 0;
	}

	return (int)m_indices->m_bufferByteSize / (int)m_indices->m_elementByteSize;
}


//-----------------------------------------------------------------------------------------------
int GPUMesh::GetVertexCount()
{
	if ( (int)m_vertices->m_elementByteSize == 0 )
	{
		return 0;
	}

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
	if ( indices.size() > 0 )
	{
		UpdateIndices( (uint)indices.size(), &indices[0] );
	}
}
