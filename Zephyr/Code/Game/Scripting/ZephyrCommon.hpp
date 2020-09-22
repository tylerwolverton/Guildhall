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
		fData = 0.f;
	}

	ZephyrValue( float value )
	{
		m_type = eValueType::NUMBER;
		fData = value;
	}

	ZephyrValue( bool value )
	{
		m_type = eValueType::BOOL;
		bData = value;
	}

	ZephyrValue( const std::string& value )
	{
		m_type = eValueType::STRING;
		strData = new std::string( value );
	}

	ZephyrValue( ZephyrValue& other )
	{
		if ( this->m_type == eValueType::STRING )
		{
			delete this->strData;
		}

		switch ( other.m_type )
		{
			case eValueType::STRING: this->strData = new std::string( *other.strData );	break;
			case eValueType::NUMBER: this->fData = other.fData;	break;
			case eValueType::BOOL: this->bData = other.bData;	break;
		}

		m_type = other.m_type;
	}

	ZephyrValue operator=( ZephyrValue& other )
	{
		if ( this->m_type == eValueType::STRING )
		{
			delete this->strData;
		}

		switch ( other.m_type )
		{
			case eValueType::STRING: this->strData = new std::string( *other.strData );	break;
			case eValueType::NUMBER: this->fData = other.fData;	break;
			case eValueType::BOOL: this->bData = other.bData;	break;
		}

		m_type = other.m_type;
	}

	~ZephyrValue()
	{
		if ( m_type == eValueType::STRING )
		{
			delete strData;
			strData = nullptr;
		}
	}

	float GetAsFloat() const { return fData; }
	bool GetAsBool() const { return bData; }
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
		float fData;
		bool bData;
		std::string* strData = nullptr;
	};
};
