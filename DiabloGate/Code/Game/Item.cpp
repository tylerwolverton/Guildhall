#include "Game/Item.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/EntityDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Item::Item( const EntityDefinition& entityDef, Map* curMap )
	: Entity( entityDef, curMap )
{
	SpriteAnimationSetDefinition* animSetDef = entityDef.GetSpriteAnimSetDef( "Default" );
	if ( animSetDef != nullptr )
	{
		m_itemSpriteDef = &( animSetDef->GetSpriteAnimationDefForDirection( Vec2::ONE )->GetSpriteDefAtTime( 0.f ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Item::RenderInUI() const
{
	Vec2 worldToUIScaleFactor = Vec2( WINDOW_WIDTH_PIXELS / WINDOW_WIDTH, WINDOW_HEIGHT_PIXELS / WINDOW_HEIGHT );

	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef == nullptr )
	{
		return;
	}

	animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( Vec2::ONE );

	const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AABB2 bounds = m_entityDef.GetLocalDrawBounds();
	bounds.SetDimensions( bounds.GetDimensions() * worldToUIScaleFactor );
	AppendVertsForAABB2D( vertexes, bounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, GetPosition() );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}
