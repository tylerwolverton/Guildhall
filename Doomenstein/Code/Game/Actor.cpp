#include "Game/Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const EntityDefinition& entityDef )
	: Entity( entityDef )
{
	m_canBePushedByWalls = true;
	m_canBePushedByEntities = true;
	m_canPushEntities = true;
}


//-----------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
		
	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Actor::Render() const
{
	Entity::Render();
}


//-----------------------------------------------------------------------------------------------
void Actor::Die()
{
	Entity::Die();
}
