#include "Game/Actor.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIElement.hpp"
#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UIUniformGrid.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/LevelThresholdDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const EntityDefinition& entityDef, Map* curMap )
	: Entity( entityDef, curMap )
{
	if ( IsNearlyEqual( m_attackSpeedModifier, 0.f ) )
	{
		m_attackSpeedModifier = 1.f;
	}
}


//
////-----------------------------------------------------------------------------------------------
//void Actor::DealDamage( EventArgs* args )
//{
//	if ( !m_canAttack )
//	{
//		return;
//	}
//
//	Entity* parentEntity = (Entity*)args->GetValue( "parentEntity", (void*)nullptr );
//
//	if ( parentEntity == nullptr
//		 || m_attackTarget == nullptr
//		 || parentEntity->GetId() != m_id )
//	{
//		return;
//	}
//
//	int damage = GetTotalAttackDamageRange().GetRandomInRange( g_game->m_rng );
//	if ( g_game->m_rng->RollPercentChance( GetTotalCritChance() ) )
//	{
//		damage *= 2;
//	}
//
//	m_attackTarget->TakeDamage( damage );
//	if ( m_attackTarget->IsDead() )
//	{
//		m_attackTarget = nullptr;
//		m_curState = eActorState::NORMAL;
//	}
//
//	m_canAttack = false;
//}
