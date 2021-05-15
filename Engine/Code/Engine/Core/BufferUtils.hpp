#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
enum class eBufferEndianMode
{
	NATIVE,
	LITTLE,
	BIG,
};


//-----------------------------------------------------------------------------------------------
eBufferEndianMode GetNativeEndianness();
void Reverse2BytesInPlace( byte* dataPtr );
void Reverse4BytesInPlace( byte* dataPtr );
void Reverse8BytesInPlace( byte* dataPtr );
