#include "Game/Scripting/ZephyrCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


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
		case eTokenType::STRING:			return "String";
		case eTokenType::FIRE_EVENT:		return "FireEvent";
		case eTokenType::ON_EVENT:			return "OnEvent";
		case eTokenType::IF:				return "if";
		case eTokenType::ELSE:				return "else";
		case eTokenType::IDENTIFIER:		return "Identifier";
		case eTokenType::CONSTANT_NUMBER:	return "Constant number";
		case eTokenType::CONSTANT_STRING:	return "Constant string";
		case eTokenType::PLUS:				return "+";
		case eTokenType::MINUS:				return "-";
		case eTokenType::STAR:				return "*";
		case eTokenType::SLASH:				return "/";
		case eTokenType::EQUAL:				return "=";
		case eTokenType::SEMICOLON:			return ";";
		case eTokenType::COMMA:				return ",";
		case eTokenType::QUOTE:				return "\"";
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
		case eTokenType::STRING:			return "STRING";
		case eTokenType::FIRE_EVENT:		return "FIRE_EVENT";
		case eTokenType::ON_EVENT:			return "ON_EVENT";
		case eTokenType::IF:				return "IF";
		case eTokenType::ELSE:				return "ELSE";
		case eTokenType::IDENTIFIER:		return "IDENTIFIER";
		case eTokenType::CONSTANT_NUMBER:	return "CONSTANT_NUMBER";
		case eTokenType::CONSTANT_STRING:	return "CONSTANT_STRING";
		case eTokenType::PLUS:				return "PLUS";
		case eTokenType::MINUS:				return "MINUS";
		case eTokenType::STAR:				return "STAR";
		case eTokenType::SLASH:				return "SLASH";
		case eTokenType::EQUAL:				return "EQUAL";
		case eTokenType::SEMICOLON:			return "SEMICOLON";
		case eTokenType::COMMA: 			return "COMMA";
		case eTokenType::QUOTE: 			return "QUOTE";
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
		case eValueType::STRING:	return "String";

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
bool ZephyrValue::IsTrue() const
{
	switch ( m_type )
	{
		case eValueType::STRING: 	return *strData != std::string( "" );
		case eValueType::NUMBER: 	return !IsNearlyEqual( numberData, 0.f );			
		case eValueType::BOOL:		return boolData;					
	}

	return false;
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
