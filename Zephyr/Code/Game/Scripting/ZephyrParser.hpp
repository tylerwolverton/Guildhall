#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;
class ZephyrToken;


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
	bool CreateBytecodeChunk( const std::string& chunkName );
	void FinalizeCurBytecodeChunk();

	bool WriteByteToCurChunk( byte newByte );
	bool WriteOpCodeToCurChunk( eOpCode opCode );
	bool WriteConstantToCurChunk( const ZephyrValue& constant );

	bool IsCurTokenType( const eTokenType& type );
	bool DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type );

	void DeclareVariable( const ZephyrToken& identifier );
	bool TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const;

	bool ParseBlock();
	bool ParseStatement();
	bool ParseNumberDeclaration();
	bool ParseFireEvent();
	bool ParseEventArgs();
	bool ParseAssignment();
	bool ParseExpression();
	bool ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel precLevel );
	bool ParseParenthesesGroup();
	bool ParseUnaryExpression();
	bool ParseBinaryExpression();
	bool ParseNumberExpression();
	bool ParseIdentifierExpressionOfType( eValueType expectedType );

	// Pratt Parser Helpers
	bool CallPrefixFunction( const ZephyrToken& token );
	bool CallInfixFunction( const ZephyrToken& token );
	eOpPrecedenceLevel GetPrecedenceLevel( const ZephyrToken& token );
	eOpPrecedenceLevel GetNextHighestPrecedenceLevel( const ZephyrToken& token );

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
	
	ZephyrBytecodeChunk* m_stateMachineBytecodeChunk = nullptr;
	std::vector<ZephyrBytecodeChunk*> m_bytecodeChunks;
	ZephyrBytecodeChunk* m_curBytecodeChunk = nullptr;
};
