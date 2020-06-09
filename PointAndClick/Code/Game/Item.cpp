#include "Game/Item.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/ItemDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Item::Item( const Vec2& position, ItemDefinition* itemDef )
	: Entity( position, (EntityDefinition*)itemDef )
	, m_itemDef( itemDef )
{

}


//-----------------------------------------------------------------------------------------------
Item::~Item()
{

}


//-----------------------------------------------------------------------------------------------
void Item::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	UpdateAnimation();
}


//-----------------------------------------------------------------------------------------------
void Item::Render() const
{
	if ( m_curAnimDef == nullptr
		 || m_isInPlayerInventory )
	{
		return;
	}

	const SpriteDefinition& spriteDef = m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_itemDef->m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, m_position );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Item::Die()
{
	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
void Item::UpdateAnimation()
{
	m_curAnimDef = m_itemDef->GetSpriteAnimDef( "Idle" );
}


