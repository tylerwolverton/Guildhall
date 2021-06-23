#include "Engine/ZephyrCore/ZephyrScanner.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/ZephyrCore/ZephyrToken.hpp"


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
			case '{': AddToken( eTokenType::BRACE_LEFT );														 break;
			case '}': AddToken( eTokenType::BRACE_RIGHT );														 break;
			case '(': AddToken( eTokenType::PARENTHESIS_LEFT );													 break;
			case ')': AddToken( eTokenType::PARENTHESIS_RIGHT );												 break;
			case '+': AddToken( eTokenType::PLUS );																 break;
			case '-': AddToken( eTokenType::MINUS );															 break;
			case '*': AddToken( eTokenType::STAR );																 break;
			case '/': AddToken( eTokenType::SLASH );															 break;
			case ';': AddToken( eTokenType::SEMICOLON );														 break;
			case ':': AddToken( eTokenType::COLON );															 break;
			case ',': AddToken( eTokenType::COMMA );															 break;
			case '.': 
			{
				if ( IsNumber( Peek() ) )
				{
					TokenizeNumberConstant();
				}
				else
				{
					AddToken( eTokenType::PERIOD );
				}
			}
			break;
			case '=': 
			case '!':
			case '>':
			case '<': TokenizeComparator( nextChar );															 break;
			case '"': TokenizeStringConstant();																	 break;
			case '|':
			case '&': TokenizeLogicalOperator( nextChar );														 break;
			case EOF: 																							 break;
			
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
				else
				{
					// This is a division slash
					return;
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
void ZephyrScanner::TokenizeComparator( char curChar )
{
	switch ( curChar )
	{
		case '=':
		{
			if ( Peek() == '=' )
			{
				AddToken( eTokenType::EQUAL_EQUAL );
				ReadAndAdvanceSrcPos();
			}
			else
			{
				AddToken( eTokenType::EQUAL );
			}
		}
		break;

		case '!':
		{
			if ( Peek() == '=' )
			{
				AddToken( eTokenType::BANG_EQUAL );
				ReadAndAdvanceSrcPos();
			}
			else
			{
				AddToken( eTokenType::BANG );
			}
		}
		break;

		case '>':
		{
			if ( Peek() == '=' )
			{
				AddToken( eTokenType::GREATER_EQUAL );
				ReadAndAdvanceSrcPos();
			}
			else
			{
				AddToken( eTokenType::GREATER );
			}
		}
		break;

		case '<':
		{
			if ( Peek() == '=' )
			{
				AddToken( eTokenType::LESS_EQUAL );
				ReadAndAdvanceSrcPos();
			}
			else
			{
				AddToken( eTokenType::LESS );
			}
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::TokenizeLogicalOperator( char curChar )
{
	char nextChar = ReadAndAdvanceSrcPos();
	if ( curChar == '|' 
		 && nextChar == '|' ) 
	{ 
		AddToken( eTokenType::OR ); 
	}
	else if ( curChar == '&'
			  && nextChar == '&' )
	{ 
		AddToken( eTokenType::AND ); 
	}
	else
	{
		AddToken( eTokenType::UNKNOWN );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::TokenizeNumberConstant()
{
	bool periodSeen = false;
	bool errorNumber = false;

	while ( IsNumber( Peek() )
			|| Peek() == '.' )
	{
		if ( Peek() == '.' )
		{
			if ( periodSeen == false )
			{
				periodSeen = true;
			}
			else
			{
				errorNumber = true;
			}
		}

		ReadAndAdvanceSrcPos();
	}

	if ( errorNumber )
	{
		int stringLength = m_curSrcPos - m_startSrcPos;
		std::string errorMsg = m_scriptSource.substr( m_startSrcPos, stringLength );
		errorMsg += " is an invalid number";
		AddToken( eTokenType::ERROR_TOKEN, errorMsg );
		return;
	}
	
	int stringLength = m_curSrcPos - m_startSrcPos;
	std::string numberConstant = m_scriptSource.substr( m_startSrcPos, stringLength );

	AddToken( eTokenType::CONSTANT_NUMBER, numberConstant );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScanner::TokenizeStringConstant()
{
	while ( Peek() != '"'
			&& Peek() != EOF )
	{
		ReadAndAdvanceSrcPos();
	}

	int strStartPos = m_startSrcPos + 1;

	int stringLength = m_curSrcPos - strStartPos;
	std::string stringConstant = m_scriptSource.substr( strStartPos, stringLength );

	AddToken( eTokenType::CONSTANT_STRING, stringConstant );

	// Consume closing " 
	ReadAndAdvanceSrcPos();
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

	// Must be identifier ( variable name or function call )
	if( !MatchReservedIdentifier( curIdentifier ) )
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


//-----------------------------------------------------------------------------------------------
bool ZephyrScanner::MatchReservedIdentifier( const std::string& identifier )
{
	switch	( MatchesReservedName( identifier, "State" ) ) 
	{  
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::STATE ); 
		case eReservedKeywordResult::CASE_MISMATCH: return true; 
	}

	switch ( MatchesReservedName( identifier, "Function" ) )
	{
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::FUNCTION );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}

	switch ( MatchesReservedName( identifier, "Number" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::NUMBER );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch ( MatchesReservedName( identifier, "Vec2" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::VEC2 );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch ( MatchesReservedName( identifier, "Bool" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::BOOL );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch ( MatchesReservedName( identifier, "String" ) )
	{
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::STRING );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}

	switch	( MatchesReservedName( identifier, "Entity" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::ENTITY );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "OnEnter" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::ON_ENTER );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "OnExit" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::ON_EXIT );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "OnUpdate" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::ON_UPDATE );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "ChangeState" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::CHANGE_STATE );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "if" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::IF );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "else" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::ELSE );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "return" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::RETURN );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "true" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::TRUE_TOKEN );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "false" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::FALSE_TOKEN );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}
	
	switch	( MatchesReservedName( identifier, "null" ) )
	{ 
		case eReservedKeywordResult::MATCH:			AddToken( eTokenType::NULL_TOKEN );
		case eReservedKeywordResult::CASE_MISMATCH: return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
eReservedKeywordResult ZephyrScanner::MatchesReservedName( const std::string& identifier, const std::string& reservedIdentifier )
{
	if ( IsEqualIgnoreCase( identifier, reservedIdentifier ) )
	{
		if ( identifier != reservedIdentifier )
		{
			std::string errorMsg = Stringf( "Error: Case mismatch, '%s' must be %s", identifier.c_str(), reservedIdentifier.c_str() );
			AddToken( eTokenType::ERROR_TOKEN, errorMsg );
			return eReservedKeywordResult::CASE_MISMATCH;
		}

		return eReservedKeywordResult::MATCH;
	}

	return eReservedKeywordResult::NO_MATCH;
}
