#include "Game/Scripting/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::string ToString( eTokenType type )
{
	switch ( type )
	{
		case eTokenType::BRACE_LEFT:		return "{";
		case eTokenType::BRACE_RIGHT:		return "}";
		case eTokenType::PARENTHESIS_LEFT:	return "(";
		case eTokenType::PARENTHESIS_RIGHT:	return ")";
		case eTokenType::STATE_MACHINE:		return "StateMachine";
		case eTokenType::STATE:				return "State";
		case eTokenType::NUMBER:			return "Number";
		case eTokenType::IDENTIFIER:		return "Identifier";
		case eTokenType::CONSTANT_NUMBER:	return "Constant number";
		case eTokenType::PLUS:				return "+";
		case eTokenType::MINUS:				return "-";
		case eTokenType::STAR:				return "*";
		case eTokenType::SLASH:				return "/";
		case eTokenType::EQUAL:				return "=";
		case eTokenType::SEMICOLON:			return ";";
		case eTokenType::END_OF_FILE:		return "End of File";
		default:							return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
std::string GetTokenName( eTokenType type )
{
	switch ( type )
	{
		case eTokenType::BRACE_LEFT:		return "BRACE_LEFT";
		case eTokenType::BRACE_RIGHT:		return "BRACE_RIGHT";
		case eTokenType::PARENTHESIS_LEFT:	return "PARENTHESIS_LEFT";
		case eTokenType::PARENTHESIS_RIGHT:	return "PARENTHESIS_RIGHT";
		case eTokenType::STATE_MACHINE:		return "STATE_MACHINE";
		case eTokenType::STATE:				return "STATE";
		case eTokenType::NUMBER:			return "NUMBER";
		case eTokenType::IDENTIFIER:		return "IDENTIFIER";
		case eTokenType::CONSTANT_NUMBER:	return "CONSTANT_NUMBER";
		case eTokenType::PLUS:				return "PLUS";
		case eTokenType::MINUS:				return "MINUS";
		case eTokenType::STAR:				return "STAR";
		case eTokenType::SLASH:				return "SLASH";
		case eTokenType::EQUAL:				return "EQUAL";
		case eTokenType::SEMICOLON:			return "SEMICOLON";
		case eTokenType::END_OF_FILE:		return "END_OF_FILE";
		default:							return "UNKNOWN";
	}
}


//-----------------------------------------------------------------------------------------------
eOpCode ByteToOpCode( byte opCodeByte )
{
	if ( opCodeByte < 0
			|| opCodeByte >= (byte)eOpCode::LAST_VAL )
	{
		return eOpCode::UNKNOWN;
	}

	return (eOpCode)opCodeByte;	
}


//-----------------------------------------------------------------------------------------------
std::string ToString( eValueType valueType )
{
	switch ( valueType )
	{
		case eValueType::NUMBER:	return "Number";
		case eValueType::BOOL:		return "Bool";
		case eValueType::STRING:	 return "String";

		case eValueType::NONE:		
		default: return "None";
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue()
{
	m_type = eValueType::NONE;
	numberData = 0.f;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( NUMBER_TYPE value )
{
	m_type = eValueType::NUMBER;
	numberData = value;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( bool value )
{
	m_type = eValueType::BOOL;
	boolData = value;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( const std::string& value )
{
	m_type = eValueType::STRING;
	strData = new std::string( value );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( ZephyrValue const& other )
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


//-----------------------------------------------------------------------------------------------
ZephyrValue::~ZephyrValue()
{
	if ( m_type == eValueType::STRING )
	{
		delete strData;
		strData = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrValue::GetAsString() const
{
	if ( strData == nullptr )
	{
		return "";
	}

	return *strData;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue& ZephyrValue::operator=( ZephyrValue const& other )
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

	return *this;
}
