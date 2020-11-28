#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerClient.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
EntityId Entity::s_nextEntityId = 0;


//-----------------------------------------------------------------------------------------------
Entity::Entity( const EntityDefinition& entityDef )
	: m_entityDef( entityDef )
{
	m_id = s_nextEntityId++;

	if ( entityDef.m_type == eEntityType::PINKY )
	{
		GetNewWanderDestination();
	}

	ChangeSpriteAnimation( "Walk" );
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	UpdateAnimation( deltaSeconds );

	if ( m_isDead )
	{
		return;
	}

	if ( m_entityDef.GetType() == eEntityType::PINKY )
	{
		if ( m_wanderTimer.HasElapsed() )
		{
			GetNewWanderDestination();
		}

		m_velocity = m_entityDef.GetWalkSpeed() * GetForwardVector();
	}

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
void Entity::UpdateAnimation( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	if ( g_playerClient == nullptr )
	{
		return;
	}

	std::vector<Vertex_PCU> vertices;
	Vec3 corners[4];

	switch ( m_entityDef.m_billboardStyle )
	{
		case eBillboardStyle::CAMERA_FACING_XY:		BillboardSpriteCameraFacingXY( m_position, m_entityDef.GetVisualSize(), *g_playerClient->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_OPPOSING_XY:	BillboardSpriteCameraOpposingXY( m_position, m_entityDef.GetVisualSize(), *g_playerClient->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_FACING_XYZ:	BillboardSpriteCameraFacingXYZ( m_position, m_entityDef.GetVisualSize(), *g_playerClient->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_OPPOSING_XYZ:	BillboardSpriteCameraOpposingXYZ( m_position, m_entityDef.GetVisualSize(), *g_playerClient->GetWorldCamera(), corners ); break;
		
		default: BillboardSpriteCameraFacingXY( m_position, m_entityDef.GetVisualSize(), *g_playerClient->GetWorldCamera(), corners ); break;
	}
	
	Vec2 mins, maxs;
	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef != nullptr )
	{
		animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_position, m_orientationDegrees, *g_playerClient->GetWorldCamera() );
	}

	// Show error image when no animation is bound
	if ( animDef == nullptr )
	{
		AppendVertsForQuad( vertices, corners, Rgba8::WHITE );

		g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/test.png" ) );
	}
	else
	{
		const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( m_cumulativeTime );
		spriteDef.GetUVs( mins, maxs );

		AppendVertsForQuad( vertices, corners, Rgba8::WHITE, mins, maxs );

		g_renderer->BindDiffuseTexture( &( spriteDef.GetTexture() ) );
	}

	g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	if ( m_isDead )
	{
		return;
	}

	m_isDead = true;
	ChangeSpriteAnimation( "Death" );
	m_cumulativeTime = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	if ( m_isPossessed )
	{
		return;
	}

	DebugAddWorldWireCylinder( Vec3( m_position, 0.f ), Vec3( m_position, m_entityDef.m_height ), m_entityDef.m_physicsRadius, Rgba8::CYAN );
}


//-----------------------------------------------------------------------------------------------
void Entity::GetNewWanderDestination()
{
	Vec2 newWanderDestination = Vec2( g_game->m_rng->RollRandomFloatInRange( 0.f, 10.f ), g_game->m_rng->RollRandomFloatInRange( 0.f, 10.f ) );
	m_wanderDirection = ( newWanderDestination - m_position ).GetNormalized();
	m_orientationDegrees = m_wanderDirection.GetOrientationDegrees();
	m_wanderTimer.SetSeconds( m_wanderSeconds );
	m_wanderTimer.Reset();
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
	
	g_playerClient->AddScreenShakeIntensity(.05f);
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


//-----------------------------------------------------------------------------------------------
void Entity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
}

