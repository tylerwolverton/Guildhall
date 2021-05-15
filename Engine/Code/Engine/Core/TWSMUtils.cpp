#include "Engine/Core/TWSMUtils.hpp"
#include "Engine/Core/BufferParser.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Core/Devconsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
void SaveMeshAsTWSMFile( const CPUMesh& mesh, const std::string& filename )
{
	std::vector<byte> fileData;
	BufferWriter bufferWriter( fileData );
	bufferWriter.AppendByte( 'T' );
	bufferWriter.AppendByte( 'W' );
	bufferWriter.AppendByte( 'S' );
	bufferWriter.AppendByte( 'M' );
	bufferWriter.AppendByte( 29 );
	bufferWriter.AppendByte( 1 );

	int numVertices = mesh.GetNumVertices();
	int numIndices = mesh.GetNumIndices();
	bufferWriter.AppendByte( 2 );
	bufferWriter.AppendUint32( sizeof( Vertex_PCUTBN ) );
	bufferWriter.AppendUint32( (uint32_t)numVertices );
	bufferWriter.AppendUint32( (uint32_t)numIndices );

	std::vector<Vertex_PCUTBN> const& vertices =  mesh.GetVertices();
	for ( int vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx )
	{
		bufferWriter.AppendVertexPCUTBN( vertices[vertexIdx] );
	}
	
	std::vector<uint> const& indices = mesh.GetIndices();
	for ( int indexIdx = 0; indexIdx < numIndices; ++indexIdx )
	{
		bufferWriter.AppendUint32( indices[indexIdx] );
	}

	WriteBufferToFile( filename, fileData.data(), (uint32_t)fileData.size() );
}


//-----------------------------------------------------------------------------------------------
CPUMesh* LoadTWSMFileIntoCPUMesh( const std::string& filename )
{
	uint32_t fileSize = 0;
	void* fileData = FileReadBinaryToNewBuffer( filename, &fileSize );

	if ( fileSize == 0 )
	{
		g_devConsole->PrintError( "Could not open ghsm file" );
		return nullptr;
	}

	BufferParser bufferParser( fileData, fileSize );
	if ( bufferParser.ParseChar() != 'T'
		 || bufferParser.ParseChar() != 'W'
		 || bufferParser.ParseChar() != 'S'
		 || bufferParser.ParseChar() != 'M' )
	{
		g_devConsole->PrintError( "Invalid File: Header CC is not GHSM" );
		return nullptr;
	}
	
	// Temp checks for testing, will handle different revisions later
	byte majorVersion = bufferParser.ParseByte();
	if ( majorVersion != 29 )
	{
		g_devConsole->PrintError( "Invalid File: Expected major version 29" );
		return nullptr;
	}

	byte minorVersion = bufferParser.ParseByte();
	if ( minorVersion != 1 )
	{
		g_devConsole->PrintWarning( "Expected minor version 1, only parsing minor version 1 things" );
	}
	
	byte vertexType = bufferParser.ParseByte();
	
	uint32_t vertexSize =  bufferParser.ParseUint32();
	uint32_t numVertices = bufferParser.ParseUint32();
	uint32_t numIndices =  bufferParser.ParseUint32();

	constexpr int HEADER_SIZE = 19;
	if ( fileSize != HEADER_SIZE + numVertices * vertexSize + numIndices * sizeof( uint ) )
	{
		g_devConsole->PrintError( "Invalid File: filesize does not match size of file calculated from header" );
		return nullptr;
	}

	uint64_t offsetToEndOfHeader = bufferParser.GetNextReadLocation() - (const byte*)fileData;

	switch ( vertexType )
	{
		case 0x01:
		{
			g_devConsole->PrintError( "Vertex format type PCU has not yet been implemented" );
			return nullptr;
		}

		case 0x02:
		{
			if ( vertexSize != sizeof( Vertex_PCUTBN ) )
			{
				g_devConsole->PrintError( Stringf( "Invalid File: Unexpected Vertex_PCUTBN size '%i' seen in file, should be '%i'", vertexSize, sizeof( Vertex_PCUTBN ) ) );
				return nullptr;
			}

			std::vector<Vertex_PCUTBN> vertices;
			vertices.resize( numVertices );
			memcpy( &vertices[0], (byte*)fileData + offsetToEndOfHeader, numVertices * vertexSize );

			std::vector<uint> indices;
			indices.resize( numIndices );
			memcpy( indices.data(), (byte*)fileData + offsetToEndOfHeader + ( numVertices * vertexSize ), numIndices * sizeof( uint ) );

			CPUMesh* mesh = new CPUMesh( vertices, indices );
			return mesh;
		}

		default:
		{
			g_devConsole->PrintError( "Invalid File: Unknown vertex format type" );
			return nullptr;
		}
	}
}
