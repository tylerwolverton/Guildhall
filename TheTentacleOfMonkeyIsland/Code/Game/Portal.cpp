#include "Game/Portal.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/PortalDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Portal::Portal( const Vec2& position, PortalDefinition* portalDef )
	: Entity( position, (EntityDefinition*)portalDef )
	, m_portalDef( portalDef )
{
}


//-----------------------------------------------------------------------------------------------
Portal::~Portal()
{

}


//-----------------------------------------------------------------------------------------------
void Portal::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Portal::Render() const
{
}


//-----------------------------------------------------------------------------------------------
void Portal::OnEnter( Actor* actor )
{
	if ( !actor->IsPlayer() )
	{
		return;
	}

	//// Check if each required item is in the player's inventory before continuing
	//for ( int itemIdx = 0; itemIdx < (int)m_requiredItemIds.size(); ++itemIdx )
	//{
	//	if ( !g_game->IsItemInInventory( m_requiredItemIds[itemIdx] ) )
	//	{
	//		return;
	//	}
	//}

	g_game->ChangeMap( m_portalDef->GetDestinationMapName() );
}

