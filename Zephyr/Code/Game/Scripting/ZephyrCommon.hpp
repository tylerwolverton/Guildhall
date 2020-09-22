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


//-----------------------------------------------------------------------------------------------
class ZephyrValue
{
public:
	ZephyrValue()
	{
		m_type = eValueType::NONE;
		numberData = 0.f;
	}

	ZephyrValue( NUMBER_TYPE value )
	{
		m_type = eValueType::NUMBER;
		numberData = value;
	}

	ZephyrValue( bool value )
	{
		m_type = eValueType::BOOL;
		boolData = value;
	}

	ZephyrValue( const std::string& value )
	{
		m_type = eValueType::STRING;
		strData = new std::string( value );
	}

	ZephyrValue( ZephyrValue const& other )
	{
		if ( this->m_type == eValueType::STRING )
		{
			delete this->strData;
		}

		switch ( other.m_type )
		{
			case eValueType::STRING: this->strData = new std::string( *other.strData );	break;
			case eValueType::NUMBER: this->numberData = other.numberData;	break;
			case eValueType::BOOL: this->boolData = other.boolData;	break;
		}

		m_type = other.m_type;
	}

	ZephyrValue operator=( ZephyrValue const& other )
	{
		if ( this->m_type == eValueType::STRING )
		{
			delete this->strData;
		}

		switch ( other.m_type )
		{
			case eValueType::STRING: this->strData = new std::string( *other.strData );	break;
			case eValueType::NUMBER: this->numberData = other.numberData;	break;
			case eValueType::BOOL: this->boolData = other.boolData;	break;
		}

		m_type = other.m_type;

		return this;
	}

	~ZephyrValue()
	{
		if ( m_type == eValueType::STRING )
		{
			delete strData;
			strData = nullptr;
		}
	}

	eValueType	GetType() const			{ return m_type; }

	float		GetAsNumber() const		{ return numberData; }
	bool		GetAsBool() const		{ return boolData; }
	std::string GetAsString() const 
	{ 
		if ( strData == nullptr )
		{
			return "";
		}
		
		return *strData;
	}

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		bool boolData;
		std::string* strData = nullptr;
	};
};
