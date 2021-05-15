#include "Engine/Core/BufferParser.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
BufferParser::BufferParser( void* dataPtr, uint64_t sizeOfData )
	: m_dataStartPtr( (byte*)dataPtr )
	, m_sizeOfData( sizeOfData )
{
	GUARANTEE_OR_DIE( m_dataStartPtr != nullptr, "Cannot initialize BufferParser with null data" );

	m_curPosition = m_dataStartPtr;
	m_endPosition = m_dataStartPtr + sizeOfData;

	m_nativeEndianMode = GetNativeEndianness();
	m_curEndianMode = m_nativeEndianMode;
}


//-----------------------------------------------------------------------------------------------
BufferParser::BufferParser( const std::vector<byte>& dataBuffer )
{
	m_dataStartPtr = &dataBuffer[0];
	m_sizeOfData = dataBuffer.size();

	m_curPosition = m_dataStartPtr;
	m_endPosition = m_dataStartPtr + m_sizeOfData;
}


//-----------------------------------------------------------------------------------------------
void BufferParser::SetEndianMode( eBufferEndianMode endianMode )
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
void BufferParser::SetReadOffset( uint32_t newOffset )
{
	//const byte* oldPosition = m_curPosition;
	m_curPosition = m_dataStartPtr + newOffset;

	GUARANTEE_OR_DIE( m_curPosition <= m_endPosition, "Requested read offset is outside the bounds of the buffer" );
}


//-----------------------------------------------------------------------------------------------
byte BufferParser::ParseByte()
{
	CheckForBufferOverrun( sizeof( byte ) );

	byte parsedVal = *m_curPosition;

	++m_curPosition;

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
bool BufferParser::ParseBool()
{
	CheckForBufferOverrun( sizeof( bool ) );

	bool parsedVal = *(bool*)&m_curPosition;

	++m_curPosition;

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
char BufferParser::ParseChar()
{
	CheckForBufferOverrun( sizeof( char ) );

	char parsedVal = *m_curPosition;

	++m_curPosition;

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
unsigned short BufferParser::ParseUshort()
{
	int dataByteLength = sizeof( unsigned short );
	CheckForBufferOverrun( dataByteLength );

	unsigned short parsedVal = *( (unsigned short*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse2BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
short BufferParser::ParseShort()
{
	int dataByteLength = sizeof( short );
	CheckForBufferOverrun( dataByteLength );

	short parsedVal = *( (short*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse2BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
uint32_t BufferParser::ParseUint32()
{
	int dataByteLength = sizeof( uint32_t );
	CheckForBufferOverrun( dataByteLength );

	uint32_t parsedVal = *( (uint32_t*)m_curPosition );

	m_curPosition += dataByteLength; 

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse4BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
int32_t BufferParser::ParseInt32()
{
	int dataByteLength = sizeof( int32_t );
	CheckForBufferOverrun( dataByteLength );

	int32_t parsedVal = *( (int32_t*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse4BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
uint64_t BufferParser::ParseUint64()
{
	int dataByteLength = sizeof( uint64_t );
	CheckForBufferOverrun( dataByteLength );

	uint64_t parsedVal = *( (uint64_t*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse8BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
int64_t BufferParser::ParseInt64()
{
	int dataByteLength = sizeof( int64_t );
	CheckForBufferOverrun( dataByteLength );

	int64_t parsedVal = *( (int64_t*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse8BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
float BufferParser::ParseFloat()
{
	int dataByteLength = sizeof( float );
	CheckForBufferOverrun( dataByteLength );

	float parsedVal = *( (float*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse4BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
double BufferParser::ParseDouble()
{
	int dataByteLength = sizeof( double );
	CheckForBufferOverrun( dataByteLength );

	double parsedVal = *( (double*)m_curPosition );

	m_curPosition += dataByteLength;

	if ( !m_doesCurEndianMatchNative )
	{
		Reverse8BytesInPlace( (byte*)&parsedVal );
	}

	return parsedVal;
}


//-----------------------------------------------------------------------------------------------
void BufferParser::ParseStringZeroTerminated( std::string& out_parsedString )
{
	const byte* startPosition = m_curPosition;

	while ( *m_curPosition != '\0' ) 
	{
		GUARANTEE_OR_DIE( m_curPosition <= m_endPosition, "Overran buffer while parsing a zero-terminated string" );

		++m_curPosition; 
	}

	out_parsedString.append( (char*)startPosition, m_curPosition - startPosition );

	// Advance past null terminator
	++m_curPosition;
}


//-----------------------------------------------------------------------------------------------
void BufferParser::ParseStringAfter32BitLength( std::string& out_parsedString )
{
	int32_t strLength = ParseInt32();

	CheckForBufferOverrun( strLength );

	out_parsedString.append( (char*)m_curPosition, strLength );

	m_curPosition += strLength;
}


//-----------------------------------------------------------------------------------------------
Vec2 BufferParser::ParseVec2()
{
	float x = ParseFloat();
	float y = ParseFloat();

	return Vec2( x, y );
}


//-----------------------------------------------------------------------------------------------
Vec3 BufferParser::ParseVec3()
{
	float x = ParseFloat();
	float y = ParseFloat();
	float z = ParseFloat();

	return Vec3( x, y, z );
}


//-----------------------------------------------------------------------------------------------
AABB2 BufferParser::ParseAABB2()
{
	Vec2 mins = ParseVec2();
	Vec2 maxs = ParseVec2();

	return AABB2( mins, maxs );
}


//-----------------------------------------------------------------------------------------------
Rgba8 BufferParser::ParseRgba8()
{
	char r = ParseChar();
	char g = ParseChar();
	char b = ParseChar();
	char a = ParseChar();

	return Rgba8( r, g, b, a );
}


//-----------------------------------------------------------------------------------------------
Vertex_PCU BufferParser::ParseVertexPCU()
{
	Vec3 position = ParseVec3();
	Rgba8 color = ParseRgba8();
	Vec2 uvs = ParseVec2();

	return Vertex_PCU( position, color, uvs );
}


//-----------------------------------------------------------------------------------------------
void BufferParser::CheckForBufferOverrun( uint64_t sizeOfRequestedData )
{	
	GUARANTEE_OR_DIE( m_curPosition + sizeOfRequestedData <= m_endPosition, Stringf( "Not enough bytes in buffer to satisfy request, requested: '%i'", sizeOfRequestedData ) );
}


