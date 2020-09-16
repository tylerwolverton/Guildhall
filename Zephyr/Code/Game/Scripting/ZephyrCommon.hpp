#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>

#define NUMBER_TYPE float

//-----------------------------------------------------------------------------------------------
enum class eTokenType
{
	UNKNOWN,
	ERROR_TOKEN,

	// Keywords
	STATE_MACHINE,
	STATE,
	NUMBER,

	BRACE_LEFT,
	BRACE_RIGHT,
	
	IDENTIFIER,

	CONSTANT_NUMBER,

	// Operators
	PLUS,
	MINUS,
	STAR,
	SLASH,
	EQUAL,
	
	SEMICOLON,

	END_OF_FILE,
};

std::string ToString( eTokenType type );


//-----------------------------------------------------------------------------------------------
enum class eOpCode : byte
{
	UNKNOWN,

	NEGATE,

	CONSTANT_NUMBER,

	ADD,
	MINUS, 
	MULTIPLY,
	DIVIDE,

	LAST_VAL,
};

eOpCode ByteToOpCode( byte opCodeByte );
