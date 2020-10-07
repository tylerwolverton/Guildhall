#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrValue;
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
#define NUMBER_TYPE float

typedef std::map<std::string, ZephyrValue> ZephyrValueMap;
typedef std::map<std::string, ZephyrBytecodeChunk*> ZephyrBytecodeChunkMap;


//-----------------------------------------------------------------------------------------------
enum class eTokenType
{
	UNKNOWN,
	ERROR_TOKEN,

	// Keywords
	STATE_MACHINE,
	STATE,
	NUMBER,
	STRING,
	FIRE_EVENT,
	ON_EVENT,
	IF,
	ELSE,

	BRACE_LEFT,
	BRACE_RIGHT,
	PARENTHESIS_LEFT,
	PARENTHESIS_RIGHT,
	
	IDENTIFIER,

	CONSTANT_NUMBER,
	CONSTANT_STRING,

	// Operators
	PLUS,
	MINUS,
	STAR,
	SLASH,
	EQUAL,
	
	// Comparators
	EQUAL_EQUAL,
	BANG,
	BANG_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,

	SEMICOLON,
	COMMA,
	QUOTE,

	END_OF_FILE,

	LAST_VAL
};

std::string ToString( eTokenType type );
std::string GetTokenName( eTokenType type );


//-----------------------------------------------------------------------------------------------
enum class eOpCode : byte
{
	UNKNOWN,

	NEGATE,
	NOT,

	CONSTANT,

	DEFINE_VARIABLE,
	GET_VARIABLE_VALUE,
	ASSIGNMENT,

	// Arithmetic
	ADD,
	SUBTRACT, 
	MULTIPLY,
	DIVIDE,

	// Comparators
	NOT_EQUAL,
	EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,

	FIRE_EVENT,

	IF,

	JUMP,

	LAST_VAL,
};

eOpCode ByteToOpCode( byte opCodeByte );


//-----------------------------------------------------------------------------------------------
enum class eValueType
{
	NONE,
	NUMBER,
	BOOL,
	STRING,
};

std::string ToString( eValueType valueType );


//-----------------------------------------------------------------------------------------------
class ZephyrValue
{
public:
	ZephyrValue();
	ZephyrValue( NUMBER_TYPE value );
	ZephyrValue( bool value );
	ZephyrValue( const std::string& value );
	ZephyrValue( ZephyrValue const& other );
	~ZephyrValue();

	ZephyrValue& operator=( ZephyrValue const& other );

	eValueType	GetType() const			{ return m_type; }

	float		GetAsNumber() const		{ return numberData; }
	bool		GetAsBool() const		{ return boolData; }
	std::string GetAsString() const;
	
	bool		IsTrue() const;

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		bool boolData;
		std::string* strData = nullptr;
	};
};
