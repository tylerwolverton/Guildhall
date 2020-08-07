#pragma once
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef unsigned char byte;


//-----------------------------------------------------------------------------------------------
enum class eOpCode : byte
{
	UNKNOWN,

	CONSTANT,
	ADD,
	SUBTRACT,
	MULTIPLY,
	DIVIDE,
	NEGATE,
	RETURN,

	LAST_VAL
};

bool operator==( eOpCode opCode, byte opCodeByte );
bool operator==( byte opCodeByte, eOpCode opCode );

byte OpCodeToByte( eOpCode opCode );
eOpCode ByteToOpCode( byte opCodeByte );

std::string ToString( eOpCode opCode );


//-----------------------------------------------------------------------------------------------
enum class eTokenType
{
	UNKNOWN,

	// Single-character tokens
	LEFT_PAREN, 
	RIGHT_PAREN,
	LEFT_BRACE, 
	RIGHT_BRACE,
	COMMA, 
	DOT, 
	MINUS, 
	PLUS,
	SEMICOLON,
	SLASH, 
	STAR,

	// One or two character tokens
	BANG, 
	BANG_EQUAL,
	EQUAL, 
	EQUAL_EQUAL,
	GREATER, 
	GREATER_EQUAL,
	LESS, 
	LESS_EQUAL,

	// Literals
	IDENTIFIER, 
	STRING, 
	NUMBER,

	// Keywords
	AND,
	OR, 
	IF,
	ELSE, 
	FALSE,
	TRUE, 
	WHILE,
	FOR,  
	PRINT, 
	RETURN, 

	ERROR,
	END_OF_FILE,

	LAST_VAL
};


//-----------------------------------------------------------------------------------------------
enum class eInterpretResult
{
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR
};


//-----------------------------------------------------------------------------------------------
union Value
{
public:
	Value( float val ) { value = val; }
	
public:
	float value;
};

//-----------------------------------------------------------------------------------------------
//template < typename T> class Value
//{
//public:
//	Value( T val ) { value = val; }
//
//public:
//	T value;
//};
