#include "Engine/Core/ObjLoader.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Time/Time.hpp"
#include "ThirdParty/mikkt/mikktspace.h"

#include <iostream>
#include <fstream>


//-----------------------------------------------------------------------------------------------
void ObjLoader::LoadFromFile( std::vector<Vertex_PCUTBN>& vertices,
							  const std::string& filename,
							  bool& out_fileHadNormals )
{
	double startTime = GetCurrentTimeSeconds();

	std::string line;
	std::ifstream objFile;
	objFile.open( filename, std::ios::in );

	if ( !objFile.is_open() )
	{
		// Load error model
		return;
	}

	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec3> uvTexCoords;
	std::vector<ObjFace> faces;
	ObjVertex lastObjVertex;
	Mat44 scaleTransform = Mat44::IDENTITY;
	OrientationMetaData orientationMetaData;
	int lineNum = 0;
	while ( std::getline( objFile, line ) )
	{
		++lineNum;

		line = TrimOuterWhitespace( line );
		
		if ( IsEmptyOrWhitespace( line ) )
		{
			continue;
		}

		Strings dataStrings = SplitStringOnDelimiter( line, ' ' );
		dataStrings = TrimOuterWhitespace( dataStrings );

		if ( dataStrings[0] == "mtllib" )
		{
			// TODO: Handle materials
		}
		else if ( dataStrings[0] == "#SquirrelMeta" )
		{
			ParseMetadata( dataStrings, lineNum, scaleTransform, orientationMetaData );
		}
		else if( dataStrings[0] == "v" )
		{
			AppendVertexData( dataStrings, positions );
		}
		else if ( dataStrings[0] == "vn" )
		{
			AppendVertexData( dataStrings, normals );
		}
		else if ( dataStrings[0] == "vt" )
		{
			AppendVertexUVs( dataStrings, uvTexCoords );
		}
		else if ( dataStrings[0] == "f" )
		{
			AppendFace( dataStrings, faces, lastObjVertex );
		}
	}

	objFile.close();

	//g_devConsole->PrintString( Stringf( "Processing obj file took: %f s", GetCurrentTimeSeconds() - startTime ) );
	startTime = GetCurrentTimeSeconds();

	for( uint faceIdx = 0; faceIdx < faces.size(); ++faceIdx )
	{
		const ObjFace& face = faces[faceIdx];
		Vertex_PCUTBN faceVertices[3];
		for ( int faceVertIdx = 0; faceVertIdx < 3; ++faceVertIdx )
		{
			Vertex_PCUTBN& vertex = faceVertices[faceVertIdx];

			int indexIntoVertArray = face.vertices[faceVertIdx].position;
			if ( indexIntoVertArray >= 0 )
			{
				vertex.position = positions[indexIntoVertArray];
			}
			else
			{
				size_t indexFromEnd = positions.size() - indexIntoVertArray;
				if ( indexFromEnd >= 0 )
				{
					vertex.position = positions[indexFromEnd];
				}
			}

			int uvIdx = face.vertices[faceVertIdx].uv;
			if( uvIdx != -1 )
			{
				vertex.uvTexCoords = uvTexCoords[uvIdx].XY();
			}

			int normalIdx = face.vertices[faceVertIdx].normal;
			if ( normalIdx != -1 )
			{
				vertex.normal = normals[normalIdx].GetNormalized();
			}
			
			vertices.push_back( vertex );
		}

		out_fileHadNormals = normals.size() != 0;
	}
	
	TransformVerts( vertices, scaleTransform );
	TransformVerts( vertices, orientationMetaData.orientationMatrix );

	if ( orientationMetaData.invertWindingOrder )
	{
		InvertVertWindingOrder( vertices );
	}

	//g_devConsole->PrintString( Stringf( "Appending verts took: %f s", GetCurrentTimeSeconds() - startTime ) );
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::AppendVertexData( const Strings& dataStrings, std::vector<Vec3>& data )
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	
	if ( dataStrings.size() > 1 )
	{
		x = (float)atof( dataStrings[1].c_str() );
	}
	if ( dataStrings.size() > 2 )
	{
		y = (float)atof( dataStrings[2].c_str() );
	}
	if ( dataStrings.size() > 3 )
	{
		z = (float)atof( dataStrings[3].c_str() );
	}

	data.push_back( Vec3( x, y, z ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::AppendVertexUVs( const Strings& dataStrings, std::vector<Vec3>& data )
{
	if ( dataStrings.size() < 3 
		 || dataStrings.size() > 4 )
	{
		g_devConsole->PrintString( Stringf( "Unsupported number of vertices: %d", dataStrings.size() - 1 ), Rgba8::YELLOW );
		return false;
	}

	float u = ConvertStringToFloat( dataStrings[1] );
	float v = ConvertStringToFloat( dataStrings[2] );

	float w = 0.f;
	if( dataStrings.size() == 4 )
	{
		w = ConvertStringToFloat( dataStrings[3] );
	}

	data.push_back( Vec3( u, v, w ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::AppendFace( const Strings& dataStrings, std::vector<ObjFace>& data, ObjVertex& lastObjVertex )
{
	if ( dataStrings.size() < 4 
		 || dataStrings.size() > 5 )
	{
		g_devConsole->PrintString( Stringf( "Unsupported number of faces: %d", dataStrings.size() - 1 ), Rgba8::YELLOW );
		return false;
	}

	ObjVertex vert0( CreateObjVertexFromString( dataStrings[1], lastObjVertex ) );
	ObjVertex vert1( CreateObjVertexFromString( dataStrings[2], lastObjVertex ) );
	ObjVertex vert2( CreateObjVertexFromString( dataStrings[3], lastObjVertex ) );

	ObjFace face0;
	face0.vertices[0] = vert0;
	face0.vertices[1] = vert1;
	face0.vertices[2] = vert2;
	
	data.push_back( face0 );

	if( dataStrings.size() == 5 )
	{
		ObjVertex vert3( CreateObjVertexFromString( dataStrings[4], lastObjVertex ) );

		ObjFace face1;
		face1.vertices[0] = vert0;
		face1.vertices[1] = vert2;
		face1.vertices[2] = vert3;

		data.push_back( face1 );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::ParseMetadata( const Strings& dataStrings, int lineNum, Mat44& scaleTransform, OrientationMetaData& orientationMetadata )
{
	int numArgs = (int)dataStrings.size() - 1;
	if ( numArgs == 0 )
	{
		g_devConsole->PrintWarning( Stringf( "Metadata on line %i has no arguments, ignoring", lineNum ) );
		return false;
	}

	if ( dataStrings[1] == "orientation" )
	{
		if ( numArgs != 4 )
		{
			g_devConsole->PrintError( Stringf( "Orientation metadata on line %i should be given in the form: orientation x=left y=up z=forward", lineNum ) );
			return false;
		}

		Vec3 newIBasis = Vec3( 1.f, 0.f, 0.f );
		Vec3 newJBasis = Vec3( 0.f, 1.f, 0.f );
		Vec3 newKBasis = Vec3( 0.f, 0.f, 1.f );
		
		float signOfOrientationChanges = 1.f;
		for ( int axisNum = 2; axisNum < 5; ++axisNum )
		{
			Strings args = SplitStringOnDelimiter( dataStrings[axisNum], '=' );
			if ( args.size() != 2 )
			{
				g_devConsole->PrintError( Stringf( "Orientation metadata on line %i should be given in the form: orientation x=left y=up z=forward", lineNum ) );
				return false;
			}

			if ( args[0] == "x" )
			{
				newIBasis = GetVecForRelativeDir( args[1] );
				signOfOrientationChanges *= ( newIBasis.x + newIBasis.y + newIBasis.z );
			}
			else if ( args[0] == "y" )
			{
				newJBasis = GetVecForRelativeDir( args[1] );
				signOfOrientationChanges *= ( newJBasis.x + newJBasis.y + newJBasis.z );
			}
			else if ( args[0] == "z" )
			{
				newKBasis = GetVecForRelativeDir( args[1] );
				signOfOrientationChanges *= ( newKBasis.x + newKBasis.y + newKBasis.z );
			}
			else
			{
				g_devConsole->PrintError( Stringf( "Orientation metadata on line %i defines unsupported axis, valid axes are x, y, and z", lineNum ) );
				return false;
			}
		}
		
		orientationMetadata.orientationMatrix.SetBasisVectors3D( newIBasis, newJBasis, newKBasis );
		orientationMetadata.invertWindingOrder = signOfOrientationChanges < 0.f;
		
	}
	else if ( dataStrings[1] == "scale" )
	{
		if ( numArgs != 2 )
		{
			g_devConsole->PrintError( Stringf( "Scale metadata on line %i should be given in the form: scale unitsPerMeter=10", lineNum ) );
			return false;
		}

		Strings args = SplitStringOnDelimiter( dataStrings[2], '=' );
		if ( args.size() != 2
			 || args[0] != "unitsPerMeter" )
		{
			g_devConsole->PrintError( Stringf( "Scale metadata on line %i should be given in the form: scale unitsPerMeter=10", lineNum ) );
			return false;
		}

		float scaleFactor = FromString( args[1], 0.f );
		if ( IsNearlyEqual( scaleFactor, 0.f )
			 || scaleFactor <= 0.f )
		{
			g_devConsole->PrintError( Stringf( "Scale factor on line %i must be a positive, non-zero value", lineNum ) );
			return false;
		}

		scaleTransform.ScaleUniform3D( 1.f / scaleFactor );
	}
	else
	{
		g_devConsole->PrintWarning( Stringf( "Unrecognized metadata type '%s' seen on line %i", dataStrings[1].c_str(), lineNum ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
Vec3 ObjLoader::GetVecForRelativeDir( const std::string& relativeDir )
{
	if ( relativeDir == "left" )	 { return Vec3( -1.f, 0.f, 0.f ); }
	if ( relativeDir == "right" )	 { return Vec3( 1.f, 0.f, 0.f ); }
	if ( relativeDir == "up" )		 { return Vec3( 0.f, 1.f, 0.f ); }
	if ( relativeDir == "down" )	 { return Vec3( 0.f, -1.f, 0.f ); }
	if ( relativeDir == "forward" )	 { return Vec3( 0.f, 0.f, 1.f ); }
	if ( relativeDir == "backward" ) { return Vec3( 0.f, 0.f, -1.f ); }

	g_devConsole->PrintError( Stringf( "Unrecognized direction, valid directions are left, right, up, down, forward, and backward" ) );

	return Vec3( 1.f, 0.f, 0.f );
}


//-----------------------------------------------------------------------------------------------
// Face entries will always be in the form v/vt/vn with both vt and vn being optional
//-----------------------------------------------------------------------------------------------
ObjVertex ObjLoader::CreateObjVertexFromString( const std::string& indexStr, ObjVertex& lastObjVertex )
{
	Strings indicesStr = SplitStringOnDelimiter( indexStr, '/' );

	ObjVertex newVertex;
	// Subtract 1 to line up the indices with arrays
	if( !indicesStr[0].empty() )
	{
		newVertex.position = ConvertStringToInt( indicesStr[0] ) - 1;
		lastObjVertex.position = newVertex.position;
	}

	// Check if we have texture coords
	if( indicesStr.size() > 1 )
	{
		if ( !indicesStr[1].empty() )
		{
			lastObjVertex.uv = ConvertStringToInt( indicesStr[1] ) - 1;
		}

		newVertex.uv = lastObjVertex.uv;
	}

	// Check if we have normals
	if ( indicesStr.size() > 2 )
	{
		if ( !indicesStr[2].empty() )
		{
			lastObjVertex.normal = ConvertStringToInt( indicesStr[2] ) - 1;
		}

		newVertex.normal = lastObjVertex.normal;
	}
		
	return newVertex;
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::InvertVertVs( std::vector<Vertex_PCUTBN>& vertices )
{
	for ( uint vertIdx = 0; vertIdx < vertices.size(); ++vertIdx )
	{
		Vertex_PCUTBN& vert = vertices[vertIdx];
		vert.uvTexCoords.y = 1.f - vert.uvTexCoords.y;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::GenerateVertNormals( std::vector<Vertex_PCUTBN>& vertices )
{
	for ( size_t vertIdx = 0; vertIdx < vertices.size(); vertIdx += 3 )
	{
		Vertex_PCUTBN& vert0 = vertices[vertIdx];
		Vertex_PCUTBN& vert1 = vertices[vertIdx + 1];
		Vertex_PCUTBN& vert2 = vertices[vertIdx + 2];

		Vec3 edge0 = vert1.position - vert0.position;
		Vec3 edge1 = vert2.position - vert0.position;

		Vec3 normal = CrossProduct3D( edge0, edge1 );
		vert0.normal = normal;
		vert1.normal = normal;
		vert2.normal = normal;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::InvertVertWindingOrder( std::vector<Vertex_PCUTBN>& vertices )
{
	for ( size_t vertIdx = 0; vertIdx < vertices.size(); vertIdx += 3 )
	{
		Vertex_PCUTBN temp = vertices[vertIdx];
		vertices[vertIdx] = vertices[vertIdx + 2];
		vertices[vertIdx + 2] = temp;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::InvertIndexWindingOrder( std::vector<uint>& indices )
{
	for ( size_t indexIdx = 0; indexIdx < indices.size(); indexIdx += 3 )
	{
		uint temp = indices[indexIdx];
		indices[indexIdx] = indices[indexIdx + 2];
		indices[indexIdx + 2] = temp;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::TransformVerts( std::vector<Vertex_PCUTBN>& vertices, const Mat44& transform )
{
	for ( size_t vertIdx = 0; vertIdx < vertices.size(); ++vertIdx )
	{
		Vertex_PCUTBN& vertex = vertices[vertIdx];
		
		vertex.position = transform.TransformPosition3D( vertex.position );

		Mat44 directionMatrix = transform.GetNormalizedDirectionMatrix3D();
		vertex.normal = directionMatrix.TransformVector3D( vertex.normal );
		vertex.tangent = directionMatrix.TransformVector3D( vertex.tangent );
		vertex.bitangent = directionMatrix.TransformVector3D( vertex.bitangent );
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::CleanMesh( std::vector<Vertex_PCUTBN>& vertices, std::vector<uint>& indices )
{
	//size_t bytesBefore = vertices.size() * sizeof( Vertex_PCUTBN );

	std::vector<Vertex_PCUTBN> uniqueVertices;
	uniqueVertices.reserve( vertices.size() );

	uint index = 0;
	for ( uint vertIdx = 0; vertIdx < vertices.size(); ++vertIdx )
	{
		bool foundVertex = false;
		for ( uint uniqueVertIdx = 0; uniqueVertIdx < uniqueVertices.size(); ++uniqueVertIdx )
		{
			// We found the vertex already, just add the index and move to next vert
			if ( uniqueVertices[uniqueVertIdx] == vertices[vertIdx] )
			{
				indices.push_back( uniqueVertIdx );

				foundVertex = true;
				break;
			}
		}

		if ( foundVertex )
		{
			continue;
		}

		uniqueVertices.push_back( vertices[vertIdx] );
		indices.push_back( index );
		++index;
	}

	vertices = uniqueVertices;

	//size_t bytesAfter = vertices.size() * sizeof( Vertex_PCUTBN ) + indices.size() * sizeof( uint );

	//float percentSavings = 100.f - ( 100.f * (float)bytesAfter / (float)bytesBefore );
	//g_devConsole->PrintString( Stringf( "Cleaned from '%d' bytes to '%d' bytes, saving %.2f percent", bytesBefore, bytesAfter, percentSavings ), Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
// Mikkelson Tangents
//-----------------------------------------------------------------------------------------------
static int GetNumFaces( SMikkTSpaceContext const* context )
{
	// if you had index buffer
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	return (int)vertices.size() / 3;
}


//-----------------------------------------------------------------------------------------------
static int GetNumberOfVerticesForFace( SMikkTSpaceContext const* context,
									   const int iFace )
{
	UNUSED( context );
	UNUSED( iFace );

	return 3;
}


//-----------------------------------------------------------------------------------------------
static void GetPositionForFaceVert( const SMikkTSpaceContext* context,
									float fvPosOut[],
									const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	Vec3 outPos = vertices[indexInVertexArray].position;

	fvPosOut[0] = outPos.x;
	fvPosOut[1] = outPos.y;
	fvPosOut[2] = outPos.z;
}


//-----------------------------------------------------------------------------------------------
static void GetNormalForFaceVert( const SMikkTSpaceContext* context,
								  float fvNormOut[],
								  const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	Vec3 outNormal = vertices[indexInVertexArray].normal;

	fvNormOut[0] = outNormal.x;
	fvNormOut[1] = outNormal.y;
	fvNormOut[2] = outNormal.z;
}


//-----------------------------------------------------------------------------------------------
static void GetUVForFaceVert( const SMikkTSpaceContext* context,
							  float fvTexcOut[],
							  const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	Vec2 outNormal = vertices[indexInVertexArray].uvTexCoords;

	fvTexcOut[0] = outNormal.x;
	fvTexcOut[1] = outNormal.y;
}


//-----------------------------------------------------------------------------------------------
static void SetTangent( const SMikkTSpaceContext* context,
						const float fvTangent[],
						const float fSign,
						const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	vertices[indexInVertexArray].tangent = Vec3( fvTangent[0], fvTangent[1], fvTangent[2] ).GetNormalized();

	vertices[indexInVertexArray].bitangent = CrossProduct3D( vertices[indexInVertexArray].tangent,
															 vertices[indexInVertexArray].normal ) * fSign;
}


//-----------------------------------------------------------------------------------------------
// Assume I'm using only vertex array - not indexed array
static void GenerateTangentsForVertexArray( std::vector<Vertex_PCUTBN>& vertices )
{
	SMikkTSpaceInterface mikktInterface;

	// How does MikkT get info
	mikktInterface.m_getNumFaces = GetNumFaces;
	mikktInterface.m_getNumVerticesOfFace = GetNumberOfVerticesForFace;

	mikktInterface.m_getPosition = GetPositionForFaceVert;
	mikktInterface.m_getNormal = GetNormalForFaceVert;
	mikktInterface.m_getTexCoord = GetUVForFaceVert;

	// MikkT telling US info
	mikktInterface.m_setTSpaceBasic = SetTangent;
	mikktInterface.m_setTSpace = nullptr;

	// Next, the context!  
	// Encapsulate ONE instance of running the algorithm
	SMikkTSpaceContext context;
	context.m_pInterface = &mikktInterface;
	context.m_pUserData = &vertices;

	// RUN THE ALGO
	genTangSpaceDefault( &context );
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::GenerateVertTangents( std::vector<Vertex_PCUTBN>& vertices )
{
	GenerateTangentsForVertexArray( vertices );
}
