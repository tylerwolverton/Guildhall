#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/BufferUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class BufferParser
{
public:
	BufferParser( void* dataPtr, uint64_t sizeOfData );
	BufferParser( const std::vector<byte>& dataBuffer );

	eBufferEndianMode GetEndianMode()										{ return m_curEndianMode; }
	void SetEndianMode( eBufferEndianMode endianMode );

	void SetReadOffset( uint32_t newOffset );

	const byte* GetNextReadLocation() const									{ return m_curPosition; }

	// Primitives
	byte ParseByte();
	bool ParseBool();
	char ParseChar();
	unsigned short ParseUshort();
	short ParseShort();
	uint32_t ParseUint32();
	int32_t ParseInt32();
	uint64_t ParseUint64();
	int64_t ParseInt64();
	float ParseFloat();
	double ParseDouble();

	// Strings
	void ParseStringZeroTerminated( std::string& out_parsedString );
	void ParseStringAfter32BitLength( std::string& out_parsedString );

	// Engine types
	Vec2 ParseVec2();
	Vec3 ParseVec3();
	AABB2 ParseAABB2();
	Rgba8 ParseRgba8();
	Vertex_PCU ParseVertexPCU();

private:
	void CheckForBufferOverrun( uint64_t sizeOfRequestedData );

private:
	const byte* m_dataStartPtr = nullptr;
	uint64_t m_sizeOfData = 0;

	const byte* m_curPosition = nullptr;
	const byte* m_endPosition = nullptr;

	eBufferEndianMode m_curEndianMode = eBufferEndianMode::LITTLE;
	eBufferEndianMode m_nativeEndianMode = eBufferEndianMode::LITTLE;
	bool m_doesCurEndianMatchNative = true;
};
