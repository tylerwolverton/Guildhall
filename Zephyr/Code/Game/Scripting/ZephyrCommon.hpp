#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>

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
	CONSTANT,

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
	NEGATE,

	CONSTANT,

	ADD,
	MINUS, 
	MULTIPLY,
	DIVIDE,
};
