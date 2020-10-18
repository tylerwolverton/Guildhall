#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"

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
	VEC2,
	STRING,
	FIRE_EVENT,
	ON_EVENT,
	CHANGE_STATE,
	IF,
	ELSE,

	BRACE_LEFT,
	BRACE_RIGHT,
	PARENTHESIS_LEFT,
	PARENTHESIS_RIGHT,
	
	IDENTIFIER,

	CONSTANT_NUMBER,
	CONSTANT_VEC2,
	CONSTANT_STRING,

	// Operators
	PLUS,
	MINUS,
	STAR,
	SLASH,
	EQUAL,
	
	// Logical Operators
	AND,
	OR,

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
	PERIOD,

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
	GET_MEMBER_VARIABLE_VALUE,
	ASSIGNMENT,
	MEMBER_ASSIGNMENT,
	ASSIGNMENT_VEC2,

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
	CHANGE_STATE,

	IF,
	JUMP,
	AND,
	OR,

	LAST_VAL,
};

eOpCode ByteToOpCode( byte opCodeByte );


//-----------------------------------------------------------------------------------------------
enum class eValueType
{
	NONE,
	NUMBER,
	VEC2,
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
	ZephyrValue( const Vec2& value );
	ZephyrValue( bool value );
	ZephyrValue( const std::string& value );
	ZephyrValue( ZephyrValue const& other );
	~ZephyrValue();

	ZephyrValue& operator=( ZephyrValue const& other );

	eValueType	GetType() const			{ return m_type; }

	float		GetAsNumber() const		{ return numberData; }
	Vec2		GetAsVec2() const		{ return vec2Data; }
	bool		GetAsBool() const		{ return boolData; }
	std::string GetAsString() const;
	
	bool		IsTrue() const;

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		Vec2 vec2Data;
		bool boolData;
		std::string* strData = nullptr;
	};
};
