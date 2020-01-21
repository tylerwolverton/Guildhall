#include "Game/Explosion.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Explosion::Explosion( Map* map, const Vec2& position, float radius, float durationSeconds )
	: Entity( map, ENTITY_TYPE_EXPLOSION, ENTITY_FACTION_NEUTRAL, position )
	, m_radius( radius )
	, m_durationSeconds( durationSeconds )
{
	m_physicsRadius = 0.f;
	m_cosmeticRadius = 0.f;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
Explosion::~Explosion()
{
	delete m_spriteAnimDef;
	m_spriteAnimDef = nullptr;

	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

//-----------------------------------------------------------------------------------------------
void Explosion::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;

	if ( m_cumulativeTime > m_durationSeconds )
	{
		Die();
	}
}


//-----------------------------------------------------------------------------------------------
void Explosion::Render() const
{
	if ( m_isDead )
	{
		return;
	}

	const SpriteDefinition& spriteDef = m_spriteAnimDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 uvAtMins, uvAtMaxs;
	spriteDef.GetUVs( uvAtMins, uvAtMaxs );

	float inverseAspect = 1.f / spriteDef.GetAspect();
	AABB2 spriteBounds( AABB2::ONE_BY_ONE );
	spriteBounds.maxs.x *= spriteDef.GetAspect();
	spriteBounds.maxs.y *= inverseAspect;

	std::vector<Vertex_PCU> vertexes;
	g_renderer->AppendVertsForAABB2D( vertexes, spriteBounds, Rgba8::WHITE, uvAtMins, uvAtMaxs );
	Vertex_PCU::TransformVertexArray( vertexes, m_radius, m_orientationDegrees, m_position );

	g_renderer->BindTexture( m_texture );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Explosion::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


//-----------------------------------------------------------------------------------------------
void Explosion::DebugRender() const
{
	Entity::DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Explosion::PopulateVertexes()
{
	m_texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Explosion_5x5.png" );
	m_spriteSheet = new SpriteSheet( *m_texture, IntVec2( 5, 5 ) );
	m_spriteAnimDef = new SpriteAnimDefinition( *m_spriteSheet, 0, 24, m_durationSeconds );

	Vec2 uvAtMins, uvAtMaxs;
	m_spriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 3 );

	g_renderer->AppendVertsForAABB2D( m_vertexes, AABB2::ONE_BY_ONE, Rgba8::WHITE, uvAtMins, uvAtMaxs );
}
