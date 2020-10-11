#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <stack>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;
class ZephyrToken;
enum class eBytecodeChunkType;

//-----------------------------------------------------------------------------------------------
enum class eOpPrecedenceLevel
{
	NONE,
	ASSIGNMENT,  // =
	OR,          // or
	AND,         // and
	EQUALITY,    // == !=
	COMPARISON,  // < > <= >=
	TERM,        // + -
	FACTOR,      // * /
	UNARY,       // ! -
	CALL,        // . ()
	PRIMARY
};


//-----------------------------------------------------------------------------------------------
class ZephyrParser
{
	friend class ZephyrCompiler;

private:
	// Private constructor so only ZephyrCompiler can use this class
	ZephyrParser( const std::string& filename, const std::vector<ZephyrToken>& tokens );
	
	ZephyrScriptDefinition* ParseTokensIntoScriptDefinition();

	// Bytecode chunk manipulation
	void CreateStateMachineBytecodeChunk();
	bool CreateBytecodeChunk( const std::string& chunkName, const eBytecodeChunkType& type );
	void FinalizeCurBytecodeChunk();

	bool WriteByteToCurChunk( byte newByte );
	bool WriteOpCodeToCurChunk( eOpCode opCode );
	bool WriteConstantToCurChunk( const ZephyrValue& constant );

	bool IsCurTokenType( const eTokenType& type );
	bool DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type );

	void DeclareVariable( const ZephyrToken& identifier, const eValueType& varType );
	bool TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const;

	bool ParseBlock();
	bool ParseStatement();
	bool ParseNumberDeclaration();
	bool ParseStringDeclaration();
	bool ParseFireEvent();
	bool ParseEventArgs();
	bool ParseChangeStateStatement();
	bool ParseIfStatement();
	bool ParseAssignment();
	bool ParseExpression( const eValueType& expressionType );
	bool ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel precLevel, const eValueType& expressionType );
	bool ParseParenthesesGroup( const eValueType& expressionType );
	bool ParseUnaryExpression( const eValueType& expressionType );
	bool ParseBinaryExpression( const eValueType& expressionType );
	bool ParseNumberExpression();
	bool ParseStringExpression();
	bool ParseIdentifierExpressionOfType( eValueType expectedType );

	// Pratt Parser Helpers
	bool CallPrefixFunction( const ZephyrToken& token, const eValueType& expressionType );
	bool CallInfixFunction( const ZephyrToken& token, const eValueType& expressionType );
	eOpPrecedenceLevel GetPrecedenceLevel( const ZephyrToken& token );
	eOpPrecedenceLevel GetNextHighestPrecedenceLevel( const ZephyrToken& token );
	eValueType GetNextValueTypeInExpression();

	void ReportError( const std::string& errorMsg );

	ZephyrToken PeekNextToken();
	ZephyrToken ConsumeNextToken();
	void AdvanceToNextToken();
	void AdvanceToNextTokenIfTypeMatches( eTokenType expectedType );
	bool ConsumeExpectedNextToken( eTokenType expectedType );
	ZephyrToken GetLastToken();
	bool IsAtEnd();

	ZephyrToken GetCurToken();
	eTokenType GetCurTokenType();
	int GetCurTokenLineNum();


private:
	std::string m_filename;
	std::vector<ZephyrToken> m_tokens;
	int m_curTokenIdx = 0;
	
	bool m_isFirstStateDef = true;
	ZephyrBytecodeChunk* m_stateMachineBytecodeChunk = nullptr;
	std::stack<ZephyrBytecodeChunk*> m_curBytecodeChunksStack;
	ZephyrBytecodeChunkMap m_bytecodeChunks;
	ZephyrBytecodeChunk* m_curBytecodeChunk = nullptr;
};
