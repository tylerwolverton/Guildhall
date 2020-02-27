#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct BufferAttribute;
class RenderContext;
class VertexBuffer;
class IndexBuffer;


//-----------------------------------------------------------------------------------------------
class GPUMesh
{
public:
	GPUMesh( RenderContext* context );
	~GPUMesh();

	void UpdateVertices( uint vertexCount, const void* vertexData, uint vertexStride, const BufferAttribute* layout );
	void UpdateIndices( uint indexCount, const uint* indices );
	void UpdateIndices( const std::vector<uint>& indices );

	// helper template
	template <typename VERTEX_TYPE>
	void UpdateVertices( uint vertexCount, const VERTEX_TYPE* vertices )
	{
		UpdateVertices( vertexCount, vertices, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT );
	}

	int GetIndexCount();
	int GetVertexCount();

public:
	VertexBuffer* m_vertices = nullptr;
	IndexBuffer* m_indices = nullptr;
};
