#include <string>
#include "Engine/Core/EngineCommon.hpp"

//-----------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( const std::string& filename, size_t* out_fileSize = nullptr );

Strings SplitFileIntoLines( const std::string& filename );
