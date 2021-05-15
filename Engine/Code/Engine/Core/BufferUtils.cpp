#include "Engine/Core/BufferUtils.hpp"


//-----------------------------------------------------------------------------------------------
eBufferEndianMode GetNativeEndianness()
{
	int32_t testInt = 0x01;

	byte* intAsByte = (byte*)&testInt;

	if ( intAsByte[0] == 0x01 )
	{
		return eBufferEndianMode::LITTLE;
	}
	else if ( intAsByte[3] == 0x01 )
	{
		return eBufferEndianMode::BIG;
	}

	ERROR_AND_DIE( "Unknown native endianness!" );
}


//-----------------------------------------------------------------------------------------------
void Reverse2BytesInPlace( byte* dataPtr )
{
	unsigned short dataAsUShort = *(unsigned short*)dataPtr;
	*(unsigned short*)dataPtr = (   ( dataAsUShort & 0x00FF ) << 8 
								  | ( dataAsUShort & 0xFF00 ) >> 8 );
}


//-----------------------------------------------------------------------------------------------
void Reverse4BytesInPlace( byte* dataPtr )
{
	int32_t dataAsUInt = *(int32_t*)dataPtr;
	*(int32_t*)dataPtr = (	( dataAsUInt & 0x000000FF ) << 24
						  | ( dataAsUInt & 0x0000FF00 ) << 8
						  | ( dataAsUInt & 0x00FF0000 ) >> 8
						  | ( dataAsUInt & 0xFF000000 ) >> 24 );
}


//-----------------------------------------------------------------------------------------------
void Reverse8BytesInPlace( byte* dataPtr )
{
	int64_t dataAsUInt = *(int64_t*)dataPtr;
	*(int64_t*)dataPtr = ( ( dataAsUInt & 0x00000000000000FF ) << 56
						 | ( dataAsUInt & 0x000000000000FF00 ) << 40
						 | ( dataAsUInt & 0x0000000000FF0000 ) << 24
						 | ( dataAsUInt & 0x00000000FF000000 ) << 8
						 | ( dataAsUInt & 0x000000FF00000000 ) >> 8
						 | ( dataAsUInt & 0x0000FF0000000000 ) >> 24
						 | ( dataAsUInt & 0x00FF000000000000 ) >> 40
						 | ( dataAsUInt & 0xFF00000000000000 ) >> 56 );
}


