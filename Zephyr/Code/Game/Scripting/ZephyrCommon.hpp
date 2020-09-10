#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>

//-----------------------------------------------------------------------------------------------
enum class eTokenType
{
	UNKNOWN,

	STATE_MACHINE,
	STATE,

	BRACE_LEFT,
	BRACE_RIGHT,
	
	NUMBER,

	PLUS,
	MINUS,
	STAR,
	SLASH,
	
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
