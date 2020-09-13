#include "Game/Scripting/ZephyrToken.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrToken::ZephyrToken( eTokenType type, const std::string& data, int lineNum )
	: m_type( type )
	, m_data( data )
	, m_lineNum( lineNum )
{
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrToken::GetDebugName() const
{
	return ToString( m_type );
}


