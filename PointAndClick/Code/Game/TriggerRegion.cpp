#include "Game/TriggerRegion.hpp"
#include "Game/Actor.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
TriggerRegion::TriggerRegion( const Vec2& position, float physicsRadius, const std::string& targetMap )
	: m_position( position )
	, m_physicsRadius( physicsRadius )
	, m_targetMap( targetMap )
{

}


//-----------------------------------------------------------------------------------------------
TriggerRegion::~TriggerRegion()
{
}


//-----------------------------------------------------------------------------------------------
void TriggerRegion::AddRequiredItem( Item* item )
{
	m_requiredItems.push_back( item );
}


//-----------------------------------------------------------------------------------------------
void TriggerRegion::OnTriggerEnter( Actor* actor )
{
	if ( !actor->IsPlayer() )
	{
		return;
	}

	// Check if each required item is in the player's inventory before continuing
	for ( int itemIdx = 0; itemIdx < (int)m_requiredItems.size(); ++itemIdx )
	{
		if ( !g_game->IsItemInInventory( m_requiredItems[itemIdx] ) )
		{
			return;
		}
	}

	g_game->ChangeMap( m_targetMap );
	//g_game->ChangeGameState( eGameState::ATTRACT );
}

