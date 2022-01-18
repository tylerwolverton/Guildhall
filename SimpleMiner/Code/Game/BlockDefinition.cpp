#include "Game/BlockDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::vector<BlockDefinition> BlockDefinition::s_blockDefs;


//-----------------------------------------------------------------------------------------------
BlockDefinition::BlockDefinition( const std::string& name )
	: m_name( name )
{
	AddToVector( *this );
}


//-----------------------------------------------------------------------------------------------
void BlockDefinition::AddToVector( const BlockDefinition& blockDef )
{
	s_blockDefs.push_back( blockDef );
}

