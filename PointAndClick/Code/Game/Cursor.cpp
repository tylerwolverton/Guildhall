#include "Game/Cursor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Cursor::Cursor( const Vec2& position, EntityDefinition* entityDef )
	: Entity( position, entityDef )
{
	m_curAnimDef = m_entityDef->GetSpriteAnimDef( "Idle" );
}


//-----------------------------------------------------------------------------------------------
Cursor::~Cursor()
{
}


//-----------------------------------------------------------------------------------------------
void Cursor::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
	m_position = g_inputSystem->GetNormalizedMouseClientPos() * g_window->GetDimensions();
}


//-----------------------------------------------------------------------------------------------
void Cursor::Render() const
{
	if ( m_curAnimDef == nullptr )
	{
		return;
	}

	const SpriteDefinition& spriteDef = m_curAnimDef->GetSpriteDefAtTime( m_cumulativeTime );
		
	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_entityDef->m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 120.f, 0.f, m_position );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Cursor::Die()
{
	Entity::Die();
}
