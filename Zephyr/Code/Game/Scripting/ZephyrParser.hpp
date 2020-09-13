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

	std::vector<ZephyrBytecodeChunk> ParseTokensIntoBytecodeChunks();

	bool IsCurTokenType( const eTokenType& type );
	bool DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type );
	bool ParseBlock();
	bool ParseStatement();
	bool ParseNumberDeclaration();
	bool ParseNumberExpression();

	void ReportError( const std::string& errorMsg );

	ZephyrToken ConsumeNextToken();
	ZephyrToken GetLastToken();
	bool IsAtEnd();

	eTokenType GetCurTokenType();
	int GetCurTokenLineNum();

private:
	bool m_isErrorFree = false;
	std::vector<ZephyrToken> m_tokens;
	std::vector<ZephyrBytecodeChunk> m_bytecodeChunks;
	int m_curTokenIdx = 0;
};
