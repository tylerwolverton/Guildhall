#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( EntityDefinition* entityDef )
	: m_entityDef( entityDef )
{
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	// vel += acceleration * dt;
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
	// pos += vel * dt;
	m_position += m_velocity * deltaSeconds;

	//update orientation
	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	ApplyFriction();
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	std::vector<Vertex_PCU> verticesCopy( m_vertices );
	Vertex_PCU::TransformVertexArray( verticesCopy, 1.f, 0.f, m_position );

	g_renderer->BindDiffuseTexture( m_texture );
	g_renderer->DrawVertexArray( verticesCopy );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	g_renderer->BindDiffuseTexture( nullptr );
	DrawRing2D( g_renderer, m_position, m_entityDef->m_physicsRadius, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damage )
{
	m_curHealth -= damage;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	g_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void Entity::ApplyFriction()
{
	if ( m_velocity.GetLength() > PHYSICS_FRICTION_FRACTION )
	{
		m_velocity -= m_velocity * PHYSICS_FRICTION_FRACTION;
	}
	else
	{
		m_velocity = Vec2( 0.f, 0.f );
	}
}
