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
		case eTokenType::STATE:				return "State";
		case eTokenType::FUNCTION:			return "Function";
		case eTokenType::VARIABLE:			return "Variable";
		case eTokenType::NUMBER:			return "Number";
		case eTokenType::VEC2:				return "Vec2";
		case eTokenType::BOOL:				return "Bool";
		case eTokenType::STRING:			return "String";
		case eTokenType::ENTITY:			return "Entity";
		case eTokenType::ON_EVENT:			return "OnEvent";
		case eTokenType::ON_ENTER:			return "OnEnter";
		case eTokenType::ON_UPDATE:			return "OnUpdate";
		case eTokenType::ON_EXIT:			return "OnExit";
		case eTokenType::CHANGE_STATE:		return "ChangeState";
		case eTokenType::IF:				return "if";
		case eTokenType::ELSE:				return "else";
		case eTokenType::RETURN:			return "return";
		case eTokenType::TRUE:				return "true";
		case eTokenType::FALSE:				return "false";
		case eTokenType::IDENTIFIER:		return "Identifier";
		case eTokenType::CONSTANT_NUMBER:	return "Constant Number";
		case eTokenType::CONSTANT_STRING:	return "Constant String";
		case eTokenType::PLUS:				return "+";
		case eTokenType::MINUS:				return "-";
		case eTokenType::STAR:				return "*";
		case eTokenType::SLASH:				return "/";
		case eTokenType::AND:				return "&&";
		case eTokenType::OR:				return "||";
		case eTokenType::EQUAL:				return "=";
		case eTokenType::EQUAL_EQUAL:		return "==";
		case eTokenType::BANG:				return "!";
		case eTokenType::BANG_EQUAL:		return "!=";
		case eTokenType::GREATER:			return ">";
		case eTokenType::GREATER_EQUAL:		return ">=";
		case eTokenType::LESS:				return "<";
		case eTokenType::LESS_EQUAL:		return "<=";
		case eTokenType::SEMICOLON:			return ";";
		case eTokenType::COMMA:				return ",";
		case eTokenType::QUOTE:				return "\"";
		case eTokenType::PERIOD:			return ".";
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
		case eTokenType::STATE:				return "STATE";
		case eTokenType::FUNCTION:			return "FUNCTION";
		case eTokenType::NUMBER:			return "NUMBER";
		case eTokenType::VEC2:				return "VEC2";
		case eTokenType::BOOL:				return "BOOL";
		case eTokenType::STRING:			return "STRING";
		case eTokenType::ENTITY:			return "ENTITY";
		case eTokenType::ON_EVENT:			return "ON_EVENT";
		case eTokenType::ON_ENTER:			return "ON_ENTER";
		case eTokenType::ON_UPDATE:			return "ON_UPDATE";
		case eTokenType::ON_EXIT:			return "ON_EXIT";
		case eTokenType::CHANGE_STATE:		return "CHANGE_STATE";
		case eTokenType::IF:				return "IF";
		case eTokenType::ELSE:				return "ELSE";
		case eTokenType::RETURN:			return "RETURN";
		case eTokenType::TRUE:				return "TRUE";
		case eTokenType::FALSE:				return "FALSE";
		case eTokenType::IDENTIFIER:		return "IDENTIFIER";
		case eTokenType::CONSTANT_NUMBER:	return "CONSTANT_NUMBER";
		case eTokenType::CONSTANT_STRING:	return "CONSTANT_STRING";
		case eTokenType::PLUS:				return "PLUS";
		case eTokenType::MINUS:				return "MINUS";
		case eTokenType::STAR:				return "STAR";
		case eTokenType::SLASH:				return "SLASH";
		case eTokenType::AND:				return "AND";
		case eTokenType::OR:				return "OR";
		case eTokenType::EQUAL:				return "EQUAL";
		case eTokenType::EQUAL_EQUAL:		return "EQUAL_EQUAL";
		case eTokenType::BANG:				return "BANG";
		case eTokenType::BANG_EQUAL:		return "BANG_EQUAL";
		case eTokenType::GREATER:			return "GREATER";
		case eTokenType::GREATER_EQUAL:		return "GREATER_EQUAL";
		case eTokenType::LESS:				return "LESS";
		case eTokenType::LESS_EQUAL:		return "LESS_EQUAL";
		case eTokenType::SEMICOLON:			return "SEMICOLON";
		case eTokenType::COMMA: 			return "COMMA";
		case eTokenType::QUOTE: 			return "QUOTE";
		case eTokenType::PERIOD: 			return "PERIOD";
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
		case eValueType::VEC2:		return "Vec2";
		case eValueType::BOOL:		return "Bool";
		case eValueType::STRING:	return "String";
		case eValueType::ENTITY:	return "Entity";

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
ZephyrValue::ZephyrValue( const Vec2& value )
{
	m_type = eValueType::VEC2;
	vec2Data = value;
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
ZephyrValue::ZephyrValue( EntityId value )
{
	m_type = eValueType::ENTITY;
	entityData = value;
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
		case eValueType::STRING:	this->strData = new std::string( *other.strData );	break;
		case eValueType::NUMBER:	this->numberData = other.numberData;	break;
		case eValueType::VEC2:		this->vec2Data = other.vec2Data;	break;
		case eValueType::BOOL:		this->boolData = other.boolData;	break;
		case eValueType::ENTITY:	this->entityData = other.entityData;	break;
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
		case eValueType::VEC2: 		return !IsNearlyEqual( vec2Data, Vec2::ZERO );			
		case eValueType::NUMBER: 	return !IsNearlyEqual( numberData, 0.f );			
		case eValueType::BOOL:		return boolData;	
		case eValueType::ENTITY:	return entityData != -1;
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
		case eValueType::STRING:	this->strData = new std::string( *other.strData );	break;
		case eValueType::NUMBER:	this->numberData = other.numberData;	break;
		case eValueType::VEC2:		this->vec2Data = other.vec2Data;	break;
		case eValueType::BOOL:		this->boolData = other.boolData;	break;
		case eValueType::ENTITY:	this->entityData = other.entityData;	break;
	}

	m_type = other.m_type;

	return *this;
}
