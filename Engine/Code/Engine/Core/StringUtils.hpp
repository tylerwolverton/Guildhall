#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
const std::string Stringv( char const* format, va_list args );
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

bool atob( const char* value );
bool atob( const std::string& value );

std::vector<std::string> SplitStringOnDelimiter( const std::string& stringToSplit, char delimiter );
std::vector<std::string> SplitStringOnDelimiterAndTrimOuterWhitespace( const std::string& stringToSplit, char delimiter );

bool IsWhitespace( unsigned char c );
std::string TrimOuterWhitespace( const std::string& stringToTrim );
bool IsEmptyOrWhitespace( const std::string& stringToCheck );

int ConvertStringToInt( const std::string& string );
Ints ConvertStringsToInts( const Strings& strings );
float ConvertStringToFloat( const std::string& string );
Floats ConvertStringsToFloats( const Strings& strings );