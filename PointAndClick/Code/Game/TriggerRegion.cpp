#include "Game/TriggerRegion.hpp"
#include "Game/Actor.hpp"
#include "Game/Item.hpp"
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
	if ( item == nullptr )
	{
		return;
	}

	m_requiredItemIds.push_back( item->GetName() );
}


//-----------------------------------------------------------------------------------------------
void TriggerRegion::AddRequiredItem( const std::string& itemId )
{
	m_requiredItemIds.push_back( itemId );
}


//-----------------------------------------------------------------------------------------------
void TriggerRegion::OnTriggerEnter( Actor* actor )
{
	if ( !actor->IsPlayer() )
	{
		return;
	}

	// Check if each required item is in the player's inventory before continuing
	for ( int itemIdx = 0; itemIdx < (int)m_requiredItemIds.size(); ++itemIdx )
	{
		if ( !g_game->IsItemInInventory( m_requiredItemIds[itemIdx] ) )
		{
			return;
		}
	}

	g_game->ChangeMap( m_targetMap );
}

