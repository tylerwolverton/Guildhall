#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrToken;


//-----------------------------------------------------------------------------------------------
class ZephyrScanner
{
	friend class ZephyrCompiler;

private:
	// Private constructor so only ZephyrCompiler can use this class
	ZephyrScanner( const std::string& scriptSource );

	std::vector<ZephyrToken> ScanSourceIntoTokens();

	void AddToken( eTokenType type, const std::string& data = "" );

	void SkipWhitespaceAndComments();
	void TokenizeNumberConstant();
	void TokenizeStringConstant();
	void TokenizeIdentifier();

	char ReadAndAdvanceSrcPos();
	char Peek();
	char PeekNextChar();
	bool IsSrcPosAtEnd();

	bool IsNumber( char c );
	bool IsLetter( char c );
	bool IsLetterOrNumber( char c );

private:
	std::string m_scriptSource;
	std::vector<ZephyrToken> m_tokens;
	int m_curLineNum = 1;
	int m_startSrcPos = 0;
	int m_curSrcPos = 0;
};
