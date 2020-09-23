#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
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

	bool IsErrorFree() const												{ return m_isErrorFree; }

	std::vector<ZephyrBytecodeChunk*> ParseTokensIntoBytecodeChunks();

	// Bytecode chunk manipulation
	bool CreateBytecodeChunk( const std::string& chunkName );
	void FinalizeCurBytecodeChunk();

	bool WriteByteToCurChunk( byte newByte );
	bool WriteOpCodeToCurChunk( eOpCode opCode );
	bool WriteConstantToCurChunk( const ZephyrValue& constant );

	bool IsCurTokenType( const eTokenType& type );
	bool DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type );
	bool ParseBlock();
	bool ParseStatement();
	bool ParseNumberDeclaration();
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
	bool ConsumeExpectedNextToken( eTokenType expectedType );
	ZephyrToken GetLastToken();
	bool IsAtEnd();

	ZephyrToken GetCurToken();
	eTokenType GetCurTokenType();
	int GetCurTokenLineNum();


private:
	std::string m_filename;
	bool m_isErrorFree = false;
	std::vector<ZephyrToken> m_tokens;
	int m_curTokenIdx = 0;
	
	std::vector<ZephyrBytecodeChunk*> m_bytecodeChunks;
	ZephyrBytecodeChunk* m_curBytecodeChunk = nullptr;
};
