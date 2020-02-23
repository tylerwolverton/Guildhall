#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
struct BufferAttribute;
class VertexBuffer;
class IndexBuffer;


//-----------------------------------------------------------------------------------------------
class GPUMesh
{
public:
	void UpdateVertices( uint vertexCount, const void* vertexData, uint vertexStride, const BufferAttribute* layout );
	void UpdateIndices( uint indexCount, const uint* indices );


	// helper template
	template <typename VERTEX_TYPE>
	void UpdateVertices( uint vertexCount, const VERTEX_TYPE* vertices )
	{
		UpdateVertices( vertexCount, vertices, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT );
	}

public:
	VertexBuffer* m_vertices;
	IndexBuffer* m_indices;
};
