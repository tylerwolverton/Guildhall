#include "Engine/Core/CPUMesh.hpp"


//-----------------------------------------------------------------------------------------------
CPUMesh::CPUMesh( const std::vector<Vertex_PCUTBN>& vertices, const std::vector<uint>& indices )
	: m_vertices( vertices )
	, m_indices( indices )
{

}


//-----------------------------------------------------------------------------------------------
CPUMesh::CPUMesh()
{

}

