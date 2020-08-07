#include "Scanner.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
Scanner::Scanner( const std::string& source )
	: m_source( source )
{

}


//-----------------------------------------------------------------------------------------------
std::vector<Token> Scanner::Scan()
{
	std::vector<Token> tokens;

	int curLine = 0;
	while ( true )
	{
		Token token = ScanNextToken();
		if ( token.GetLineNum() != curLine )
		{
			std::cout << token.GetLineNum();
			curLine = token.GetLineNum();
		}
		else
		{
			std::cout << "    | ";
		}

		std::cout << (int)token.GetType() << " '" << token.GetStart() << "'\n";

		tokens.push_back( token );

		if ( token.GetType() == eTokenType::END_OF_FILE )
		{
			break;
		}
	}

	return tokens;
}


//-----------------------------------------------------------------------------------------------
Token Scanner::ScanNextToken()
{
	SkipWhitespaceAndComments();

	m_startIdx = m_curIdx;

	if ( IsAtEnd() )
	{
		return CreateToken( eTokenType::END_OF_FILE );
	}

	char nextChar = Advance();

	switch ( nextChar )
	{
		case '(': return CreateToken( eTokenType::LEFT_PAREN );
		case ')': return CreateToken( eTokenType::RIGHT_PAREN );
		case '{': return CreateToken( eTokenType::LEFT_BRACE );
		case '}': return CreateToken( eTokenType::RIGHT_BRACE );
		case ';': return CreateToken( eTokenType::SEMICOLON );
		case ',': return CreateToken( eTokenType::COMMA );
		case '.': return CreateToken( eTokenType::DOT );
		case '-': return CreateToken( eTokenType::MINUS );
		case '+': return CreateToken( eTokenType::PLUS );
		case '/': return CreateToken( eTokenType::SLASH );
		case '*': return CreateToken( eTokenType::STAR );

		case '>': AdvanceIfMatch( '=' ) ? CreateToken( eTokenType::GREATER_EQUAL ) : CreateToken( eTokenType::GREATER );
		case '<': AdvanceIfMatch( '=' ) ? CreateToken( eTokenType::LESS_EQUAL ) : CreateToken( eTokenType::LESS );
		case '=': AdvanceIfMatch( '=' ) ? CreateToken( eTokenType::EQUAL_EQUAL ) : CreateToken( eTokenType::EQUAL );
		case '!': AdvanceIfMatch( '=' ) ? CreateToken( eTokenType::BANG_EQUAL ) : CreateToken( eTokenType::BANG );
		
	}

	return CreateToken( eTokenType::ERROR );
}


//-----------------------------------------------------------------------------------------------
Token Scanner::CreateToken( eTokenType type )
{
	return Token( type, &m_source[m_startIdx], m_curIdx - m_startIdx, m_curLineNum );
}


//-----------------------------------------------------------------------------------------------
void Scanner::SkipWhitespaceAndComments()
{
	while ( true )
	{
		char nextChar = Peek();
		switch ( nextChar )
		{
			case ' ':
			case '\r':
			case '\t':
			{
				Advance();
			}
			break;

			case '\n':
			{
				++m_curLineNum;
				Advance();
			}
			break;

			case '/':
			{
				if ( PeekNext() == '/' )
				{
					// A comment goes until the end of the line.
					while ( Peek() != '\n' && !IsAtEnd() )
					{
						Advance();
					}
				}
				else
				{
					return;
				}
			}
			break;

			default:
				return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Scanner::IsAtEnd()
{
	return m_curIdx >= (int)m_source.size();
}


//-----------------------------------------------------------------------------------------------
char Scanner::Advance()
{
	return m_source[m_curIdx++];
}


//-----------------------------------------------------------------------------------------------
bool Scanner::AdvanceIfMatch( char charToMatch )
{
	if ( IsAtEnd() 
		 || Peek() != charToMatch )
	{
		return false;
	}

	Advance();
	return true;
}


//-----------------------------------------------------------------------------------------------
char Scanner::Peek()
{
	return m_source[m_curIdx];
}


//-----------------------------------------------------------------------------------------------
char Scanner::PeekNext()
{
	if ( IsAtEnd() )
	{
		return '\0';
	}

	return m_source[m_curIdx + 1];
}

