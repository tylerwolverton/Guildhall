#include <string>
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( const std::string& filename, size_t* out_fileSize = nullptr );

Strings SplitFileIntoLines( const std::string& filename );
Strings GetFileNamesInFolder( const std::string& relativeFolderPath, const char* filePattern );
std::string GetFileName( const std::string& filePath );
std::string GetFileNameWithoutExtension( const std::string& filePath );
