#include "Game/Item.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
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
	m_cumulativeTime += deltaSeconds;
	if ( m_isInPlayerInventory )
	{
		return;
	}

	UpdateAnimation();
}


//-----------------------------------------------------------------------------------------------
void Item::Render() const
{
	if ( m_isInPlayerInventory )
	{
		return;
	}

	if ( m_curAnimDef != nullptr )
	{
		const SpriteDefinition& spriteDef = m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime );

		Vec2 mins, maxs;
		spriteDef.GetUVs( mins, maxs );

		std::vector<Vertex_PCU> vertexes;
		AppendVertsForAABB2D( vertexes, m_itemDef->m_localDrawBounds, Rgba8::WHITE, mins, maxs );

		Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, m_position );

		g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
		g_renderer->DrawVertexArray( vertexes );
	}
	else if ( m_texture != nullptr )
	{
		std::vector<Vertex_PCU> vertexes;
		AppendVertsForAABB2D( vertexes, m_itemDef->m_localDrawBounds, Rgba8::WHITE );

		Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, m_position );

		g_renderer->BindTexture( 0, m_texture );
		g_renderer->DrawVertexArray( vertexes );
	}
}


//-----------------------------------------------------------------------------------------------
void Item::Die()
{
	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
SpriteDefinition* Item::GetSpriteDef() const
{
	if ( m_curAnimDef == nullptr )
	{
		return nullptr;
	}

	return const_cast<SpriteDefinition*>( &( m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime ) ) );
}


//-----------------------------------------------------------------------------------------------
void Item::AddVerbState( eVerbState verbState, NamedProperties* properties )
{
	auto mapIter = m_itemDef->m_verbPropertiesMap.find( verbState );
	if ( mapIter != m_itemDef->m_verbPropertiesMap.end() )
	{
		m_itemDef->m_verbPropertiesMap.erase( verbState );
	}

	m_itemDef->m_verbPropertiesMap[verbState] = properties;
}


//-----------------------------------------------------------------------------------------------
void Item::RemoveVerbState( eVerbState verbState )
{
	m_itemDef->m_verbPropertiesMap.erase( verbState );
}


//-----------------------------------------------------------------------------------------------
void Item::HandleVerbAction( eVerbState verbState )
{
	NamedProperties* verbEventProperties = m_itemDef->GetVerbEventProperties( verbState );

	if( verbEventProperties == nullptr )
	{
		if ( m_itemDef->GetName() == "Wooden Arms"
			 && verbState == eVerbState::PICKUP )
		{
			g_game->PrintTextOverPlayer( "I should ask the cook before taking that" );
		}
		else
		{
			g_game->PrintTextOverPlayer( "Hmmm, that's not going to work" );
		}

		g_game->ClearCurrentActionText();

		return;
	}

	EventArgs args;
	args.SetValue( "target", (void*)this );
	args.SetValue( "properties", (void*)verbEventProperties );

	std::string verbEventName = verbEventProperties->GetValue( "eventName", "" );
	if ( verbEventName == "" )
	{
		g_devConsole->PrintError( Stringf( "Invalid event name '%s' seen for verb state '%s'", verbEventName.c_str(), GetEventNameForVerbState( verbState ).c_str() ) );
		return;
	}

	g_eventSystem->FireEvent( verbEventName, &args );
}


//-----------------------------------------------------------------------------------------------
void Item::UpdateAnimation()
{
	if ( m_velocity.x > 0.05f )
	{
		m_curAnimDef = m_itemDef->GetSpriteAnimDef( "MoveEast" );
	}
	else if ( m_velocity.x < -0.05f )
	{
		m_curAnimDef = m_itemDef->GetSpriteAnimDef( "MoveWest" );
	}
	else if ( m_velocity.y > 0.05f )
	{
		m_curAnimDef = m_itemDef->GetSpriteAnimDef( "MoveNorth" );
	}
	else if ( m_velocity.y < -0.05f )
	{
		m_curAnimDef = m_itemDef->GetSpriteAnimDef( "MoveSouth" );
	}
	else
	{
		m_curAnimDef = m_itemDef->GetSpriteAnimDef( "Idle" );
	}
}
