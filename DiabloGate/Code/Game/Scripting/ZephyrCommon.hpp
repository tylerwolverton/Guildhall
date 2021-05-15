#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrValue;
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
#define NUMBER_TYPE float

typedef std::map<std::string, ZephyrValue> ZephyrValueMap;
typedef std::map<std::string, ZephyrBytecodeChunk*> ZephyrBytecodeChunkMap;

constexpr int ERROR_ZEPHYR_VAL = -1000;
extern std::string PARENT_ENTITY_NAME;


//-----------------------------------------------------------------------------------------------
enum class eTokenType
{
	UNKNOWN,
	ERROR_TOKEN,

	// Keywords
	STATE,
	FUNCTION,
	NUMBER,
	VEC2,
	BOOL,
	STRING,
	ENTITY,
	ON_ENTER,
	ON_UPDATE,
	ON_EXIT,
	CHANGE_STATE,
	IF,
	ELSE,
	RETURN,
	TRUE_TOKEN,
	FALSE_TOKEN,
	NULL_TOKEN,

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
	COLON,
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
	CONSTANT_VEC2,

	DEFINE_VARIABLE,
	GET_VARIABLE_VALUE,
	ASSIGNMENT,
	MEMBER_ASSIGNMENT,
	MEMBER_ACCESSOR,
	MEMBER_FUNCTION_CALL,

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

	FUNCTION_CALL,
	CHANGE_STATE,

	RETURN,

	IF,
	JUMP,
	AND,
	OR,

	LAST_VAL,
};

eOpCode ByteToOpCode( byte opCodeByte );
std::string ToString( eOpCode opCode );

//-----------------------------------------------------------------------------------------------
enum class eValueType
{
	NONE,
	NUMBER,
	VEC2,
	BOOL,
	STRING,
	ENTITY,
};

std::string ToString( eValueType valueType );


//-----------------------------------------------------------------------------------------------
struct EntityVariableInitializer
{
public:
	std::string varName;
	std::string entityName;

public:
	EntityVariableInitializer( const std::string& varName, const std::string& entityName )
		: varName( varName )
		, entityName( entityName )
	{
	}
};


//-----------------------------------------------------------------------------------------------
class ZephyrValue
{
public:
	ZephyrValue();
	ZephyrValue( NUMBER_TYPE value );
	ZephyrValue( const Vec2& value );
	ZephyrValue( bool value );
	ZephyrValue( const std::string& value );
	ZephyrValue( EntityId value );
	ZephyrValue( ZephyrValue const& other );
	~ZephyrValue();

	ZephyrValue& operator=( ZephyrValue const& other );

	eValueType	GetType() const			{ return m_type; }

	float		GetAsNumber() const		{ return numberData; }
	Vec2		GetAsVec2() const		{ return vec2Data; }
	bool		GetAsBool() const		{ return boolData; }
	std::string GetAsString() const;
	EntityId	GetAsEntity() const		{ return entityData; }
	
	bool		EvaluateAsBool();
	Vec2		EvaluateAsVec2();
	std::string	EvaluateAsString();
	float		EvaluateAsNumber();
	EntityId	EvaluateAsEntity();

private:
	void ReportConversionError( eValueType targetType );

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		Vec2 vec2Data;
		bool boolData;
		std::string* strData = nullptr;
		EntityId entityData;
	};
};
