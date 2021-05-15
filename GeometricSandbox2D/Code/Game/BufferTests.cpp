#include "Game/BufferTests.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Core/BufferParser.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
void BufferTests::RunTests()
{
	std::vector<byte> buffer;

	BufferWriter writer( buffer );
	//writer.SetEndianMode( eBufferEndianMode::BIG );
	writer.AppendByte( 'T' );
	writer.AppendByte( 'Y' );
	writer.AppendByte( 'L' );
	writer.AppendByte( 'E' );
	writer.AppendByte( 'R' );
	writer.AppendUshort( 65000 );
	writer.AppendShort( -32000 );
	writer.AppendUint32( 4294967292 );
	writer.AppendInt32( -2147483640 );
	writer.AppendUint64( 18446744073709551614 );
	writer.AppendInt64( -9223372036854775800 );
	writer.AppendFloat( 1.f );
	writer.AppendDouble( 3.1415926535897932384626433832795 );
	
	writer.AppendStringZeroTerminated( "Hello" );
	writer.AppendStringAfter32BitLength( "I'll never be terminated!" );

	writer.AppendVec2( Vec2( 1.f, 1.f ) );
	writer.AppendVec3( Vec3( 1.f, 1.f, 1.f ) );
	writer.AppendAABB2( AABB2( Vec2( 1.f, 1.f ), Vec2( 12.f, 12.f ) ) );
	writer.AppendRgba8( Rgba8( 128, 128, 255, 100 ) );
	writer.AppendVertexPCU( Vertex_PCU( Vec3( 0.f, 0.f, 1.f ), Rgba8( 128, 128, 255, 100 ), Vec2( 1.f, 1.f ) ) );

	GUARANTEE_OR_DIE( buffer[0] == 'T', "T not found in buffer" );
	GUARANTEE_OR_DIE( buffer[1] == 'Y', "Y not found in buffer" );
	GUARANTEE_OR_DIE( buffer[2] == 'L', "L not found in buffer" );
	GUARANTEE_OR_DIE( buffer[3] == 'E', "E not found in buffer" );
	GUARANTEE_OR_DIE( buffer[4] == 'R', "R not found in buffer" );
	//GUARANTEE_OR_DIE( buffer[5] == 7, "Ushort not saved as 7" );

	BufferParser parser( buffer );
	//parser.SetEndianMode( eBufferEndianMode::BIG );
	byte t = parser.ParseByte();
	parser.SetReadOffset( 0 );
	t = parser.ParseByte();
	byte y = parser.ParseByte();
	byte l = parser.ParseByte();
	byte e = parser.ParseByte();
	byte r = parser.ParseByte();
	unsigned short ushortVal = parser.ParseUshort();
	short shortVal = parser.ParseShort();
	uint32_t uint32Val = parser.ParseUint32();
	int32_t int32Val = parser.ParseInt32();
	uint64_t uint64Val = parser.ParseUint64();
	int64_t int64Val = parser.ParseInt64();
	float floatVal = parser.ParseFloat();
	double doubleVal = parser.ParseDouble();

	std::string helloStr, noTerminationStr;
	parser.ParseStringZeroTerminated( helloStr );
	parser.ParseStringAfter32BitLength( noTerminationStr );

	Vec2 vec2Val = parser.ParseVec2();
	Vec3 vec3Val = parser.ParseVec3();
	AABB2 aabb2Val = parser.ParseAABB2();
	Rgba8 rgba8Val = parser.ParseRgba8();
	Vertex_PCU vertexPCUVal = parser.ParseVertexPCU();

	//byte uhOh = parser.ParseByte();

	GUARANTEE_OR_DIE( t == 'T', "T not parsed from buffer" );
	GUARANTEE_OR_DIE( y == 'Y', "Y not parsed from buffer" );
	GUARANTEE_OR_DIE( l == 'L', "L not parsed from buffer" );
	GUARANTEE_OR_DIE( e == 'E', "E not parsed from buffer" );
	GUARANTEE_OR_DIE( r == 'R', "R not parsed from buffer" );
	GUARANTEE_OR_DIE( ushortVal == 65000, "ushort not parsed from buffer" );
	GUARANTEE_OR_DIE( shortVal == -32000, "short not parsed from buffer" );
	GUARANTEE_OR_DIE( uint32Val == 4294967292, "uint32 not parsed from buffer" );
	GUARANTEE_OR_DIE( int32Val == -2147483640, "int32 not parsed from buffer" );
	GUARANTEE_OR_DIE( uint64Val == 18446744073709551614, "uint64 not parsed from buffer" );
	GUARANTEE_OR_DIE( int64Val == -9223372036854775800, "int64 not parsed from buffer" );
	GUARANTEE_OR_DIE( floatVal == 1.f, "float not parsed from buffer" );
	GUARANTEE_OR_DIE( doubleVal == 3.1415926535897931, "double not parsed from buffer" );
	GUARANTEE_OR_DIE( helloStr == "Hello", "Zero-terminated string wasn't parsed correctly" );
	GUARANTEE_OR_DIE( noTerminationStr == "I'll never be terminated!", "String after 32 bit length wasn't parsed correctly" );
	GUARANTEE_OR_DIE( vec2Val == Vec2( 1.f, 1.f ), "Vec2 wasn't parsed correctly" );
	GUARANTEE_OR_DIE( vec3Val == Vec3( 1.f, 1.f, 1.f ), "Vec3 wasn't parsed correctly" );
	GUARANTEE_OR_DIE( IsNearlyEqual( aabb2Val, AABB2( Vec2( 1.f, 1.f ), Vec2( 12.f, 12.f ) ) ), "AABB2 wasn't parsed correctly" );
	GUARANTEE_OR_DIE( rgba8Val == Rgba8( 128, 128, 255, 100 ), "Rgba8 wasn't parsed correctly" );
	GUARANTEE_OR_DIE( IsNearlyEqual( vertexPCUVal.m_position, Vec3( 0.f, 0.f, 1.f ) ) , "Vertex_PCU position wasn't parsed correctly" );
	GUARANTEE_OR_DIE( vertexPCUVal.m_color == Rgba8( 128, 128, 255, 100 ), "Vertex_PCU color wasn't parsed correctly" );
	GUARANTEE_OR_DIE( IsNearlyEqual( vertexPCUVal.m_uvTexCoords, Vec2( 1.f, 1.f ) ), "Vertex_PCU uvs wasn't parsed correctly" );
}

