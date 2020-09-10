#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
struct IntVec2;
struct Vec3;


//-----------------------------------------------------------------------------------------------
const std::string Stringv( char const* format, va_list args );
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

bool atob( const char* value );
bool atob( const std::string& value );

std::vector<std::string> SplitStringOnDelimiter( const std::string& stringToSplit, char delimiter );

bool IsWhitespace( unsigned char c );
bool IsEmptyOrWhitespace( const std::string& stringToCheck );

std::string TrimOuterWhitespace( const std::string& stringToTrim );
Strings TrimOuterWhitespace( const Strings& stringsToTrim );

int ConvertStringToInt( const std::string& string );
Ints ConvertStringsToInts( const Strings& strings );
float ConvertStringToFloat( const std::string& string );
Floats ConvertStringsToFloats( const Strings& strings );

//-----------------------------------------------------------------------------------------------
std::string ToString( bool value );
std::string ToString( int value );
std::string ToString( uint value );
std::string ToString( float value );
std::string ToString( const std::string& value );
std::string ToString( const Rgba8& value );
std::string ToString( const Vec2& value );
std::string ToString( const Vec3& value );
std::string ToString( const IntVec2& value );

bool		FromString( const std::string& value, bool defaultValue );
int			FromString( const std::string& value, int defaultValue );
uint		FromString( const std::string& value, uint defaultValue );
float		FromString( const std::string& value, float defaultValue );
double		FromString( const std::string& value, double defaultValue );
std::string	FromString( const std::string& value, std::string defaultValue );
std::string	FromString( const std::string& value, const char* defaultValue );
Rgba8		FromString( const std::string& value, const Rgba8& defaultValue );
Vec2		FromString( const std::string& value, const Vec2& defaultValue );
Vec3		FromString( const std::string& value, const Vec3& defaultValue );
IntVec2		FromString( const std::string& value, const IntVec2& defaultValue );
void*		FromString( const std::string& value, void* defaultValue );
