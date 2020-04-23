#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <stdarg.h>
#include <cctype>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringv( char const* format, va_list args )
{
	char buffer[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s( buffer, STRINGF_STACK_LOCAL_TEMP_LENGTH, format, args );
	return buffer;
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
bool atob( const char* value )
{
	if ( !_stricmp( value, "T" )
		 || !_stricmp( value, "true" )
		 || !_stricmp( value, "1" ) )
	{
		return true;
	}
	else if ( !_stricmp( value, "F" )
			|| !_stricmp( value, "false" )
			|| !_stricmp( value, "0" ) )
	{
		return false;
	}

	ERROR_AND_DIE( Stringf( "Tried to convert '%s' to bool!", value ) );
}


//-----------------------------------------------------------------------------------------------
bool atob( const std::string& value )
{
	return atob( value.c_str() );
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> SplitStringOnDelimiter( const std::string& stringToSplit, char delimiter )
{
	std::vector<std::string> splitStrings;

	size_t startPos = 0;
	size_t foundPos = 0;
	while ( foundPos != std::string::npos )
	{
		foundPos = stringToSplit.find( delimiter, startPos );

		splitStrings.push_back( stringToSplit.substr( startPos, foundPos - startPos ) );
		startPos = foundPos + 1;
	}

	return splitStrings;
}


//-----------------------------------------------------------------------------------------------
bool IsWhitespace( unsigned char c )
{
	return std::isspace( c ) != 0;
}


//-----------------------------------------------------------------------------------------------
bool IsEmptyOrWhitespace( const std::string& stringToCheck )
{
	if ( stringToCheck.size() == 0 )
	{
		return true;
	}

	for ( uint stringIdx = 0; stringIdx < stringToCheck.size() - 1; ++stringIdx )
	{
		if ( !IsWhitespace( stringToCheck[stringIdx] ) )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
std::string TrimOuterWhitespace( const std::string& stringToTrim )
{
	if ( stringToTrim.size() == 0 )
	{
		return "";
	}

	size_t startPos = 0;
	
	while ( startPos < stringToTrim.size() - 1
			&& IsWhitespace( stringToTrim[startPos] ) )
	{
		++startPos;
	}  

	size_t endPos = stringToTrim.size() - 1;

	while ( endPos > startPos
			&& IsWhitespace( stringToTrim[endPos] ) )
	{
		--endPos;
	}
	
	// Move endpos back to the first whitespace char
	++endPos;
	return stringToTrim.substr( startPos, endPos - startPos );
}


//-----------------------------------------------------------------------------------------------
Strings TrimOuterWhitespace( const Strings& stringsToTrim )
{
	Strings trimmedStrings;
	trimmedStrings.reserve( stringsToTrim.size() );

	for( uint stringIdx = 0; stringIdx < stringsToTrim.size(); ++stringIdx )
	{
		std::string trimmedStr = TrimOuterWhitespace( stringsToTrim[stringIdx] );
		if ( trimmedStr != "" )
		{
			trimmedStrings.push_back( trimmedStr );
		}
	}

	return trimmedStrings;
}




//-----------------------------------------------------------------------------------------------
int ConvertStringToInt( const std::string& string )
{
	return atoi( string.c_str() );
}


//-----------------------------------------------------------------------------------------------
Ints ConvertStringsToInts( const Strings& strings )
{
	Ints ints;
	ints.reserve( strings.size() );

	for ( uint stringIdx = 0; stringIdx < strings.size() - 1; ++stringIdx )
	{
		ints.push_back( ConvertStringToInt( strings[stringIdx].c_str() ) );
	}

	return ints;
}


//-----------------------------------------------------------------------------------------------
float ConvertStringToFloat( const std::string& string )
{
	return (float)atof( string.c_str() );
}


//-----------------------------------------------------------------------------------------------
Floats ConvertStringsToFloats( const Strings& strings )
{
	Floats floats;
	floats.reserve( strings.size() );

	for ( uint stringIdx = 0; stringIdx < strings.size() - 1; ++stringIdx )
	{
		floats.push_back( ConvertStringToFloat( strings[stringIdx].c_str() ) );
	}

	return floats;
}
