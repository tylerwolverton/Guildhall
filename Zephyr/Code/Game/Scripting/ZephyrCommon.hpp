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
	PARENTHESIS_LEFT,
	PARENTHESIS_RIGHT,
	
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

	LAST_VAL
};

std::string ToString( eTokenType type );
std::string GetTokenName( eTokenType type );


//-----------------------------------------------------------------------------------------------
enum class eOpCode : byte
{
	UNKNOWN,

	NEGATE,

	CONSTANT_NUMBER,

	DEFINE_VARIABLE,
	GET_VARIABLE_VALUE,
	ASSIGNMENT,

	ADD,
	SUBTRACT, 
	MULTIPLY,
	DIVIDE,

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
	

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		bool boolData;
		std::string* strData = nullptr;
	};
};
