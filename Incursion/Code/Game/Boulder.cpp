#include "Game/Boulder.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Boulder::Boulder( Map* map, EntityFaction faction, const Vec2& position )
	: Entity( map, ENTITY_TYPE_BOULDER, faction, position )
{
	m_physicsRadius = BOULDER_PHYSICS_RADIUS;
	m_cosmeticRadius = BOULDER_COSMETIC_RADIUS;

	m_pushesEntities = true;
	m_isPushedByEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
Boulder::~Boulder()
{
	delete m_extrasSpriteSheet;
	m_extrasSpriteSheet = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Boulder::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Boulder::Render() const
{
	if ( IsDead() )
	{
		return;
	}

	std::vector<Vertex_PCU> vertexesCopy( m_vertexes );
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, m_orientationDegrees, m_position );

	g_renderer->BindTexture( m_texture );
	g_renderer->DrawVertexArray( vertexesCopy );
}


//-----------------------------------------------------------------------------------------------
void Boulder::Die()
{
	m_isGarbage = true;
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
void Boulder::PopulateVertexes()
{
	m_texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Extras_4x4.png" );
	m_extrasSpriteSheet = new SpriteSheet( *m_texture, IntVec2( 4, 4 ) );

	Vec2 uvAtMins, uvAtMaxs;
	m_extrasSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 3 );

	g_renderer->AppendVertsForAABB2D( m_vertexes, AABB2::ONE_BY_ONE, Rgba8::WHITE, uvAtMins, uvAtMaxs );
}
