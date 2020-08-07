#pragma once
#include "Token.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Scanner
{
public:
	Scanner( const std::string& source );

	std::vector<Token> Scan();

private:
	Token ScanNextToken();
	Token CreateToken( eTokenType type );

	void SkipWhitespaceAndComments();
	bool IsAtEnd();

	char Advance();
	bool AdvanceIfMatch( char charToMatch );
	char Peek();
	char PeekNext();

private:
	std::string m_source;
	int m_startIdx = 0;
	int m_curIdx = 0;
	/*const char* m_startChar = nullptr;
	const char* m_curChar = nullptr;*/
	int m_curLineNum = 1;
};
