#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


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
