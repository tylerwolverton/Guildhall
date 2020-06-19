#include "Game/Item.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
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
	m_curAnimDef = m_itemDef->GetSpriteAnimDef( "Idle" );
}


//-----------------------------------------------------------------------------------------------
Item::~Item()
{

}


//-----------------------------------------------------------------------------------------------
void Item::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if ( m_isInPlayerInventory )
	{
		return;
	}

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
SpriteDefinition* Item::GetSpriteDef() const
{
	return const_cast<SpriteDefinition*>( &( m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime ) ) );
}


//-----------------------------------------------------------------------------------------------
void Item::HandleVerbAction( eVerbState verbState )
{
	switch ( verbState )
	{
		case eVerbState::PICKUP:
		{
			std::string typeStr = m_itemDef->m_pickupEventArgs.GetValue( "type", "" );
			if ( typeStr == "" )
			{
				g_devConsole->PrintString( "Hmm, that's not going to work", Rgba8::ORANGE );
				return;
			}

			EventArgs args;
			args.SetValue( "targetItem", (void*)this );
			g_eventSystem->FireEvent( "OnPickUpItem", &args );
		}
		break;

		default:
		{
			g_devConsole->PrintString( "Hmm, that's not going to work", Rgba8::ORANGE );
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Item::UpdateAnimation()
{
	m_curAnimDef = m_itemDef->GetSpriteAnimDef( "Idle" );
}


