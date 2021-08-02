#include "Engine/Core/HashUtils.hpp"


//-----------------------------------------------------------------------------------------------
uint32_t Hash( byte* data, size_t count )
{
	uint32_t hash = 0U;
	for ( int byteIdx = 0; byteIdx < count; ++byteIdx )
	{
		hash *= 31;
		hash += data[byteIdx];
	}

	return hash;
}
