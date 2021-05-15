#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/BufferUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class BufferWriter
{
public:
	BufferWriter( std::vector<byte>& buffer );

	void SetEndianMode( eBufferEndianMode endianMode );
	eBufferEndianMode GetEndianMode()								{ return m_curEndianMode; }

	// Primitives
	void AppendByte( byte newByte );
	void AppendChar( char newChar );
	void AppendBool( bool newBool );
	void AppendUshort( unsigned short newUshort );
	void AppendShort( short newShort );
	void AppendUint32( uint32_t newUint32 );
	void AppendInt32( int32_t newInt32 );
	void AppendUint64( uint64_t newUint64 );
	void AppendInt64( int64_t newInt64 );
	void AppendFloat( float newFloat );
	void AppendDouble( double newDouble );

	// Strings
	void AppendStringZeroTerminated( const std::string& newString );
	void AppendStringZeroTerminated( const char* newString );
	void AppendStringAfter32BitLength( const char* newString );

	// Engine types
	void AppendVec2( const Vec2& newVec2 );
	void AppendVec3( const Vec3& newVec3 );
	void AppendAABB2( const AABB2& newAABB2 );
	void AppendRgba8( const Rgba8& newRgba8 );
	void AppendVertexPCU( const Vertex_PCU& newVertexPCU );
	void AppendVertexPCUTBN( const Vertex_PCUTBN& newVertexPCUTBN );

	uint32_t GetBufferLength() const;
	void OverwriteInt32AtOffset( int32_t newInt32, uint32_t offsetToOverwrite );
	void OverwriteUint32AtOffset( uint32_t newUint32, uint32_t offsetToOverwrite );

private:
	// Methods to intentionally write specific sizes to buffer
	void Append2BytesToBuffer( byte* dataPtr );
	void Append4BytesToBuffer( byte* dataPtr );
	void Append8BytesToBuffer( byte* dataPtr );
	   
private:
	std::vector<byte>&	m_buffer;

	eBufferEndianMode m_curEndianMode = eBufferEndianMode::LITTLE;
	eBufferEndianMode m_nativeEndianMode = eBufferEndianMode::LITTLE;
	bool m_doesCurEndianMatchNative = true;
};
