#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
BufferWriter::BufferWriter( std::vector<byte>& buffer )
	: m_buffer( buffer )
{
	m_nativeEndianMode = GetNativeEndianness();
	m_curEndianMode = m_nativeEndianMode;
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::SetEndianMode( eBufferEndianMode endianMode )
{
	if ( endianMode == eBufferEndianMode::NATIVE )
	{
		m_curEndianMode = m_nativeEndianMode;
		m_doesCurEndianMatchNative = true;
		return;
	}
	
	m_curEndianMode = endianMode;
	m_doesCurEndianMatchNative = m_curEndianMode == m_nativeEndianMode;
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendByte( byte newByte )
{
	m_buffer.push_back( newByte );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendChar( char newChar )
{
	m_buffer.push_back( newChar );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendBool( bool newBool )
{
	m_buffer.push_back( newBool );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendUshort( unsigned short newUshort )
{
	byte* dataAsBytes = (byte*)&newUshort;
	
	Append2BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendShort( short newShort )
{
	byte* dataAsBytes = (byte*)&newShort;

	Append2BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendUint32( uint32_t newUint32 )
{
	byte* dataAsBytes = (byte*)&newUint32;
	
	Append4BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendInt32( int32_t newInt32 )
{
	byte* dataAsBytes = (byte*)&newInt32;
	
	Append4BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendUint64( uint64_t newUint64 )
{
	byte* dataAsBytes = (byte*)&newUint64;

	Append8BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendInt64( int64_t newInt64 )
{
	byte* dataAsBytes = (byte*)&newInt64;
	
	Append8BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendFloat( float newFloat )
{
	byte* dataAsBytes = (byte*)&newFloat;
	
	Append4BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendDouble( double newDouble )
{
	byte* dataAsBytes = (byte*)&newDouble;
	
	Append8BytesToBuffer( dataAsBytes );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendStringZeroTerminated( const std::string& newString )
{
	AppendStringZeroTerminated( newString.c_str() );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendStringZeroTerminated( const char* newString )
{
	GUARANTEE_OR_DIE( newString != nullptr, "Nullptr passed to BufferWriter::AppendStringZeroTerminated" );

	int strIdx = 0;
	char c = newString[strIdx];
	while ( c != '\0' )
	{
		m_buffer.push_back( c );

		++strIdx;
		c = newString[strIdx];
	}

	// Push terminator
	m_buffer.push_back( '\0' );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendStringAfter32BitLength( const char* newString )
{
	GUARANTEE_OR_DIE( newString != nullptr, "Nullptr passed to BufferWriter::AppendStringAfter32BitLength" );
	
	int32_t stringSizeOffset = (int32_t)m_buffer.size();
	AppendInt32( 0 );

	int strIdx = 0;
	char c = newString[strIdx];
	while ( c != '\0' )
	{
		m_buffer.push_back( c );

		++strIdx;
		c = newString[strIdx];
	}

	OverwriteInt32AtOffset( strIdx, stringSizeOffset );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendVec2( const Vec2& newVec2 )
{
	AppendFloat( newVec2.x );
	AppendFloat( newVec2.y );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendVec3( const Vec3& newVec3 )
{
	AppendFloat( newVec3.x );
	AppendFloat( newVec3.y );
	AppendFloat( newVec3.z );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendAABB2( const AABB2& newAABB2 )
{
	AppendVec2( newAABB2.mins );
	AppendVec2( newAABB2.maxs );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendRgba8( const Rgba8& newRgba8 )
{
	AppendChar( newRgba8.r );
	AppendChar( newRgba8.g );
	AppendChar( newRgba8.b );
	AppendChar( newRgba8.a );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendVertexPCU( const Vertex_PCU& newVertexPCU )
{
	AppendVec3( newVertexPCU.m_position );
	AppendRgba8( newVertexPCU.m_color );
	AppendVec2( newVertexPCU.m_uvTexCoords );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::AppendVertexPCUTBN( const Vertex_PCUTBN& newVertexPCUTBN )
{
	AppendVec3( newVertexPCUTBN.position );
	AppendRgba8( newVertexPCUTBN.color );
	AppendVec2( newVertexPCUTBN.uvTexCoords );

	AppendVec3( newVertexPCUTBN.tangent );
	AppendVec3( newVertexPCUTBN.bitangent );
	AppendVec3( newVertexPCUTBN.normal );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::Append2BytesToBuffer( byte* dataPtr )
{
	if ( !m_doesCurEndianMatchNative )
	{
		Reverse2BytesInPlace( dataPtr );
	}

	m_buffer.push_back( *dataPtr );
	m_buffer.push_back( *( dataPtr + 1 ) );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::Append4BytesToBuffer( byte* dataPtr )
{
	if ( !m_doesCurEndianMatchNative )
	{
		Reverse4BytesInPlace( dataPtr );
	}

	m_buffer.push_back( *dataPtr );
	m_buffer.push_back( *( dataPtr + 1 ) );
	m_buffer.push_back( *( dataPtr + 2 ) );
	m_buffer.push_back( *( dataPtr + 3 ) );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::Append8BytesToBuffer( byte* dataPtr )
{
	if ( !m_doesCurEndianMatchNative )
	{
		Reverse8BytesInPlace( dataPtr );
	}

	m_buffer.push_back( *dataPtr );
	m_buffer.push_back( *( dataPtr + 1 ) );
	m_buffer.push_back( *( dataPtr + 2 ) );
	m_buffer.push_back( *( dataPtr + 3 ) );
	m_buffer.push_back( *( dataPtr + 4 ) );
	m_buffer.push_back( *( dataPtr + 5 ) );
	m_buffer.push_back( *( dataPtr + 6 ) );
	m_buffer.push_back( *( dataPtr + 7 ) );
}


//-----------------------------------------------------------------------------------------------
uint32_t BufferWriter::GetBufferLength() const
{
	return (uint32_t)m_buffer.size();
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::OverwriteInt32AtOffset( int32_t newInt32, uint32_t offsetToOverwrite )
{
	GUARANTEE_OR_DIE( offsetToOverwrite + 3 < m_buffer.size(), Stringf( "Tried to update an int32 at buffer index '%i', but buffer is only '%i' bytes long", offsetToOverwrite, m_buffer.size() ) );

	byte* dataPtr = (byte*)&newInt32;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse4BytesInPlace( dataPtr );
	}

	m_buffer[offsetToOverwrite] = ( *dataPtr );
	m_buffer[offsetToOverwrite + 1] = ( *( dataPtr + 1 ) );
	m_buffer[offsetToOverwrite + 2] = ( *( dataPtr + 2 ) );
	m_buffer[offsetToOverwrite + 3] = ( *( dataPtr + 3 ) );
}


//-----------------------------------------------------------------------------------------------
void BufferWriter::OverwriteUint32AtOffset( uint32_t newUint32, uint32_t offsetToOverwrite )
{
	GUARANTEE_OR_DIE( offsetToOverwrite + 3 < m_buffer.size(), Stringf( "Tried to update a uint32 at buffer index '%i', but buffer is only '%i' bytes long", offsetToOverwrite, m_buffer.size() ) );

	byte* dataPtr = (byte*)&newUint32;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse4BytesInPlace( dataPtr );
	}

	m_buffer[offsetToOverwrite] = ( *dataPtr );
	m_buffer[offsetToOverwrite + 1] = ( *( dataPtr + 1 ) );
	m_buffer[offsetToOverwrite + 2] = ( *( dataPtr + 2 ) );
	m_buffer[offsetToOverwrite + 3] = ( *( dataPtr + 3 ) );
}

