#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrToken;


//-----------------------------------------------------------------------------------------------
class ZephyrParser
{
	friend class ZephyrCompiler;

private:
	// Private constructor so only ZephyrCompiler can use this class
	ZephyrParser( const std::vector<ZephyrToken>& tokens );

	std::vector<ZephyrBytecodeChunk*> ParseTokensIntoBytecodeChunks();

	// Bytecode chunk manipulation
	bool CreateBytecodeChunk( const std::string& chunkName );
	void FinalizeCurBytecodeChunk();

	bool WriteByteToCurChunk( byte newByte );
	bool WriteOpCodeToCurChunk( eOpCode opCode );
	bool WriteNumberConstantToCurChunk( NUMBER_TYPE numConstant );

	bool IsCurTokenType( const eTokenType& type );
	bool DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type );
	bool ParseBlock();
	bool ParseStatement();
	bool ParseNumberDeclaration();
	bool ParseNumberExpression( NUMBER_TYPE& out_result );

	void ReportError( const std::string& errorMsg );

	ZephyrToken ConsumeNextToken();
	ZephyrToken GetLastToken();
	bool IsAtEnd();

	eTokenType GetCurTokenType();
	int GetCurTokenLineNum();

private:
	bool m_isErrorFree = false;
	std::vector<ZephyrToken> m_tokens;
	int m_curTokenIdx = 0;

	std::vector<ZephyrBytecodeChunk*> m_bytecodeChunks;
	ZephyrBytecodeChunk* m_curBytecodeChunk = nullptr;
};
