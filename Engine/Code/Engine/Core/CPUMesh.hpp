#pragma once
#include "Engine/Core/Vertex_PCUTBN.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class CPUMesh
{
public:
	CPUMesh();
	CPUMesh( const std::vector<Vertex_PCUTBN>& vertices, const std::vector<uint>& indices );

	std::vector<Vertex_PCUTBN> GetVertices() const								{ return m_vertices; }
	std::vector<uint> GetIndices() const										{ return m_indices; }

	int GetNumVertices() const													{ return (int)m_vertices.size(); }
	int GetNumIndices() const													{ return (int)m_indices.size(); }

	void SetVertices( const std::vector<Vertex_PCUTBN>& vertices )				{ m_vertices = vertices; }
	void SetIndices( const std::vector<uint>& indices )							{ m_indices = indices; }

private:
	std::vector<Vertex_PCUTBN> m_vertices;
	std::vector<uint> m_indices;
};
