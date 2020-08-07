#include "Token.hpp"


//-----------------------------------------------------------------------------------------------
Token::Token( eTokenType type, const char* start, int length, int lineNum )
	: m_type( type )
	, m_start( start )
	, m_length( length )
	, m_lineNum( lineNum )
{
}
