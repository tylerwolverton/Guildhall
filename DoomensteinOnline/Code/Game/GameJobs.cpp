#include "Game/GameJobs.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


//-----------------------------------------------------------------------------------------------
// TestJob
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void TestJob::Execute()
{
	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	MeshImportOptions importOptions;
	importOptions.generateNormals = true;
	importOptions.generateTangents = true;
	importOptions.transform = Mat44::CreateUniformScale3D( .9f );
	AppendVertsAndIndicesForObjMeshFromFile( vertices, indices, "Data/Models/scifi_fighter/mesh.obj", importOptions );
}


//-----------------------------------------------------------------------------------------------
void TestJob::ClaimJobCallback()
{
	std::string msg = Stringf( "Claiming job %i", m_id );
	g_devConsole->PrintString( msg, Rgba8::PURPLE );
}
