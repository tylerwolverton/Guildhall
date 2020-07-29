#include "Game/Portal.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/EntityDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Portal::Portal( const Vec2& position, EntityDefinition* portalDef )
	: Entity( position, portalDef )
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

	g_game->ChangeMap( m_entityDef->GetDestinationMapName() );
}

