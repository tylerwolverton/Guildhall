#include "Game/NpcTurret.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Map.hpp"
#include "RaycastImpact.hpp"


//-----------------------------------------------------------------------------------------------
NpcTurret::NpcTurret( Map* map, EntityFaction faction, const Vec2& position )
	: Entity( map, ENTITY_TYPE_NPC_TURRET, faction, position )
{
	m_physicsRadius = TURRET_PHYSICS_RADIUS;
	m_cosmeticRadius = TURRET_COSMETIC_RADIUS;
	m_maxTurnSpeed = TURRET_SEARCH_MAX_TURN_SPEED;
	m_maxHealth = m_health = TURRET_MAX_HEALTH;

	m_diedSound = g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	m_hitSound = g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	m_shootSound = g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );

	m_pushesEntities = true;
	m_isPushedByEntities = false;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
NpcTurret::~NpcTurret()
{
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::Update( float deltaSeconds )
{
	if ( IsDead() )
	{
		return;
	}

	switch ( m_aiState )
	{
		case TurretAIState::SEARCH:
			Search( deltaSeconds );
			break;
		case TurretAIState::ATTACK:
			Attack( deltaSeconds );
			break;
	}
	
	if ( m_shotCooldown > 0.f )
	{
		m_shotCooldown -= deltaSeconds;
	}

	Entity::Update( deltaSeconds );

	m_lastRaycastImpact = m_map->VisionRaycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees ), TURRET_MAX_ATTACK_RANGE );
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::Render() const
{
	if ( IsDead() )
	{
		return;
	}

	// Base
	std::vector<Vertex_PCU> vertexesCopy( m_vertexes );
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, 0.f, m_position );

	g_renderer->BindDiffuseTexture( m_turretBase );
	g_renderer->DrawVertexArray( vertexesCopy );

	// Turret
	vertexesCopy = m_vertexes;
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, m_orientationDegrees, m_position );

	g_renderer->BindDiffuseTexture( m_turretTop );
	g_renderer->DrawVertexArray( vertexesCopy );

	// Laser sight
	g_renderer->BindDiffuseTexture( nullptr );
	DrawLine2D( g_renderer, m_position + GetForwardVector() * TURRET_COSMETIC_RADIUS, m_lastRaycastImpact.m_impactPosition, Rgba8::RED, DEBUG_LINE_THICKNESS );

	RenderHealthBar();
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::DebugRender() const
{
	if ( m_isDead )
	{
		return;
	}

	// Draw attack view range
	RaycastImpact impact1 = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees + TURRET_SHOT_ANGLE_RANGE_DEGREES ), TURRET_MAX_ATTACK_RANGE );
	RaycastImpact impact2 = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees - TURRET_SHOT_ANGLE_RANGE_DEGREES ), TURRET_MAX_ATTACK_RANGE );
	DrawLine2D( g_renderer, m_position, impact1.m_impactPosition, Rgba8::GREEN, DEBUG_LINE_THICKNESS );
	DrawLine2D( g_renderer, m_position, impact2.m_impactPosition, Rgba8::GREEN, DEBUG_LINE_THICKNESS );

	Entity::DebugRender();
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::Die()
{
	m_isGarbage = true;
	m_isDead = true;

	g_audioSystem->PlaySound( m_diedSound );
	m_map->SpawnExplosion( m_position, 1.25f, 1.f );
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::TakeDamage( int damage )
{
	Entity::TakeDamage( damage );

	g_audioSystem->PlaySound( m_hitSound );
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::PopulateVertexes()
{
	m_turretBase = g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	m_turretTop = g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyCannon.png" );

	AppendVertsForAABB2D( m_vertexes, AABB2::ONE_BY_ONE, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::Search( float deltaSeconds )
{
	Entity* target = m_map->GetPlayer();

	if ( target != nullptr
		 && !target->IsDead()
		 && CanSeeTarget( *target, TURRET_MAX_ATTACK_RANGE ) )
	{
		m_aiState = TurretAIState::ATTACK;
		m_angularVelocity = 0.f;
		return;
	}

	if ( !m_hasSeenTarget )
	{
		m_angularVelocity = TURRET_SEARCH_MAX_TURN_SPEED;
	}
	else
	{
		TurnToward( m_goalOrientation, deltaSeconds );
		
		// Hit the edge of our search sector, rotate the other way
		if ( m_orientationDegrees == m_goalOrientation )
		{
			if ( m_angleToTargetLastLocationDegrees > m_orientationDegrees )
			{
				m_goalOrientation += TURRET_SEARCH_SECTOR_APERTURE;
			}
			else
			{
				m_goalOrientation -= TURRET_SEARCH_SECTOR_APERTURE;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void NpcTurret::Attack( float deltaSeconds )
{
	Entity* target = m_map->GetPlayer();

	if ( target == nullptr
		 || target->IsDead()
		 || !CanSeeTarget(*target, TURRET_MAX_ATTACK_RANGE) )
	{
		m_aiState = TurretAIState::SEARCH;
		m_hasSeenTarget = true;
		m_goalOrientation = m_angleToTargetLastLocationDegrees + TURRET_SEARCH_SECTOR_APERTURE *.5f;
		return;
	}

	Vec2 displacementToTarget = target->GetPosition() - m_position;
	float angleDisplacementToTargetDegrees = displacementToTarget.GetOrientationDegrees();

	if ( m_shotCooldown <= 0.f
		  && IsPointInForwardSector2D( target->GetPosition(), m_position, m_orientationDegrees, TURRET_SHOT_ANGLE_RANGE_DEGREES * 2.f, TURRET_MAX_ATTACK_RANGE ) )
	{
		m_map->SpawnNewEntity( ENTITY_TYPE_EVIL_BULLET, m_faction, m_position, angleDisplacementToTargetDegrees );
		m_shotCooldown = TURRET_SHOT_COOLDOWN;

		g_audioSystem->PlaySound( m_shootSound );
	}
	
	TurnToward( angleDisplacementToTargetDegrees, deltaSeconds );

	m_angleToTargetLastLocationDegrees = angleDisplacementToTargetDegrees;
}
