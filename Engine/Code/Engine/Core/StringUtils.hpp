#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
const std::string Stringv( char const* format, va_list args );
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

bool atob( const char* value );
bool atob( const std::string& value );

std::vector<std::string> SplitStringOnDelimiter( const std::string& stringToSplit, char delimiter );
