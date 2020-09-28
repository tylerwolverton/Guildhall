#include "Game/Scripting/ZephyrScanner.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrToken.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScanner::ZephyrScanner( const std::string& scriptSource )
	: m_scriptSource( scriptSource )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::AddToken( eTokenType type, const std::string& data )
{
	m_tokens.push_back( ZephyrToken( type, data, m_curLineNum ) );
}


//-----------------------------------------------------------------------------------------------
std::vector<ZephyrToken> ZephyrScanner::ScanSourceIntoTokens()
{
	std::string curWord;

	while( !IsSrcPosAtEnd() )
	{
		SkipWhitespaceAndComments();

		// Advance beginning of this token to current position before processing
		m_startSrcPos = m_curSrcPos;

		char nextChar = ReadAndAdvanceSrcPos();
		switch ( nextChar )
		{
			// Compare against known token types
			case '{': AddToken( eTokenType::BRACE_LEFT );			break;
			case '}': AddToken( eTokenType::BRACE_RIGHT );			break;
			case '(': AddToken( eTokenType::PARENTHESIS_LEFT );		break;
			case ')': AddToken( eTokenType::PARENTHESIS_RIGHT );	break;
			case '+': AddToken( eTokenType::PLUS );					break;
			case '-': AddToken( eTokenType::MINUS );				break;
			case '*': AddToken( eTokenType::STAR );					break;
			case '/': AddToken( eTokenType::SLASH );				break;
			case '=': AddToken( eTokenType::EQUAL );				break;
			case ';': AddToken( eTokenType::SEMICOLON );			break;
			case ',': AddToken( eTokenType::COMMA );				break;
			case EOF: 												break;
			
			default:
			{
				if ( IsNumber( nextChar ) )
				{
					TokenizeNumberConstant();
				}
				else if ( IsLetter( nextChar ) )
				{
					TokenizeIdentifier();
				}
				else
				{
					// TODO: Throw error
					AddToken( eTokenType::UNKNOWN );
				}
			}
			break;
		}
	}

	//ParseAndAddToTokenList( tokens, curWord, curLineNum );
	AddToken( eTokenType::END_OF_FILE );

	return m_tokens;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::SkipWhitespaceAndComments()
{
	bool parsingComment = false;

	while ( true )
	{
		if ( parsingComment )
		{
			if ( Peek() == '\n'
				 || Peek() == EOF )
			{
				parsingComment = false;
			}
			else
			{
				ReadAndAdvanceSrcPos();
				continue;
			}
		}

		switch ( Peek() )
		{
			case ' ':
			case '\t':
			case '\r':
			{
				ReadAndAdvanceSrcPos();
			}
			break;

			case '\n':
			{
				++m_curLineNum;
				ReadAndAdvanceSrcPos();
			}
			break;

			// Comment
			case '/':
			{
				if ( PeekNextChar() == '/' )
				{
					parsingComment = true;
				}
			}
			break;

			// Found a non-whitespace char, return
			default:
				return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::TokenizeNumberConstant()
{
	while ( IsNumber( Peek() ) )
	{
		ReadAndAdvanceSrcPos();
	}

	int stringLength = m_curSrcPos - m_startSrcPos;
	std::string numberConstant = m_scriptSource.substr( m_startSrcPos, stringLength );

	AddToken( eTokenType::CONSTANT_NUMBER, numberConstant );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::TokenizeIdentifier()
{
	while ( IsLetterOrNumber( Peek() ) )
	{
		ReadAndAdvanceSrcPos();
	}

	int stringLength = m_curSrcPos - m_startSrcPos;
	std::string curIdentifier = m_scriptSource.substr( m_startSrcPos, stringLength );

	// Match keywords
	if ( curIdentifier == "StateMachine" )	 { AddToken( eTokenType::STATE_MACHINE ); }
	else if ( curIdentifier == "State" )	 { AddToken( eTokenType::STATE ); }
	else if ( curIdentifier == "Number" )	 { AddToken( eTokenType::NUMBER ); }
	else if ( curIdentifier == "FireEvent" ) { AddToken( eTokenType::FIRE_EVENT ); }
	else if ( curIdentifier == "OnEvent" ) { AddToken( eTokenType::ON_EVENT ); }
	// Must be a variable name
	else
	{
		AddToken( eTokenType::IDENTIFIER, curIdentifier );
	}
}


//-----------------------------------------------------------------------------------------------
char ZephyrScanner::ReadAndAdvanceSrcPos()
{
	if ( IsSrcPosAtEnd() )
	{
		return EOF;
	}

	return m_scriptSource[m_curSrcPos++];
}


//-----------------------------------------------------------------------------------------------
char ZephyrScanner::Peek()
{
	if ( IsSrcPosAtEnd() )
	{
		return EOF;
	}

	return m_scriptSource[m_curSrcPos];
}


//-----------------------------------------------------------------------------------------------
char ZephyrScanner::PeekNextChar()
{
	if ( IsSrcPosAtEnd() )
	{
		return EOF;
	}

	return m_scriptSource[(size_t)m_curSrcPos + 1];
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScanner::IsSrcPosAtEnd()
{
	if ( m_curSrcPos >= (int)m_scriptSource.size() )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScanner::IsNumber( char c )
{
	// TODO: Can a number start with '.'?
	return c >= '0' && c <= '9';
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScanner::IsLetter( char c )
{
	// Consider underscores letters for variable names
	return ( c >= 'a' && c <='z'
			 || c >= 'A' && c <= 'Z'
			 || c == '_' );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScanner::IsLetterOrNumber( char c )
{
	return IsNumber( c ) || IsLetter( c );
}


