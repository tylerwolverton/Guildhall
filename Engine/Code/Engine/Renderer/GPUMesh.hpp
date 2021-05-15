#pragma once
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct BufferAttribute;
class RenderContext;


//-----------------------------------------------------------------------------------------------
class GPUMesh
{
public:
	~GPUMesh();

	// Templated constructor to support different vertex types
	template <typename VERTEX_TYPE>
	GPUMesh( RenderContext* context, const std::vector<VERTEX_TYPE>& vertices, const std::vector<uint>& indices )
	{
		m_context = context;
		m_vertices = new VertexBuffer( context, MEMORY_HINT_DYNAMIC, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT );
		m_indices = new IndexBuffer( context, MEMORY_HINT_DYNAMIC );

		UpdateVertices( vertices );
		UpdateIndices( indices );
	}

	GPUMesh( RenderContext* context, const CPUMesh& cpuMesh);

	void SetFromCPUMesh( const CPUMesh& cpuMesh );

	// Set up buffers
	void UpdateVertices( uint vertexCount, const void* vertexData, uint vertexStride, const BufferAttribute* layout );
	void UpdateIndices( uint indexCount, const uint* indices );
	void UpdateIndices( const std::vector<uint>& indices );

	// helper templates
	template <typename VERTEX_TYPE>
	void UpdateVertices( const std::vector<VERTEX_TYPE> vertices )
	{
		UpdateVertices( (uint)vertices.size(), &vertices[0] );
	}

	template <typename VERTEX_TYPE>
	void UpdateVertices( uint vertexCount, const VERTEX_TYPE* vertices )
	{
		UpdateVertices( vertexCount, vertices, sizeof( VERTEX_TYPE ), VERTEX_TYPE::LAYOUT );
	}

	// Accessors
	int GetIndexCount();
	int GetVertexCount();

public:
	VertexBuffer* m_vertices = nullptr;
	IndexBuffer* m_indices = nullptr;
	RenderContext* m_context = nullptr;
};
