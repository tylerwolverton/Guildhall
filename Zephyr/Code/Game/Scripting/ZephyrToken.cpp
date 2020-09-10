#include "Game/Scripting/ZephyrToken.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrToken::ZephyrToken( eTokenType type, int lineNum )
	: m_type( type )
	, m_lineNum( lineNum )
{
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrToken::GetDebugName() const
{
	return ToString( m_type );
}


