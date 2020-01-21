#include "Game/NpcTank.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
NpcTank::NpcTank( Map* map, EntityFaction faction, const Vec2& position )
	: Entity( map, ENTITY_TYPE_NPC_TANK, faction, position )
{
	m_physicsRadius = TANK_PHYSICS_RADIUS;
	m_cosmeticRadius = TANK_COSMETIC_RADIUS;
	m_maxTurnSpeed = TANK_MAX_TURN_SPEED;
	m_maxHealth = m_health = TANK_MAX_HEALTH;

	m_diedSound = g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyDied.wav" );
	m_hitSound = g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyHit.wav" );
	m_shootSound = g_audioSystem->CreateOrGetSound( "Data/Audio/EnemyShoot.wav" );

	m_pushesEntities = true;
	m_isPushedByEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
NpcTank::~NpcTank()
{
}


//-----------------------------------------------------------------------------------------------
void NpcTank::Update( float deltaSeconds )
{
	if ( m_isDead )
	{
		return;
	}

	switch ( m_aiState )
	{
		case TankAIState::WANDER:
			Wander( deltaSeconds );
			break;
		case TankAIState::CHASE:
			ChaseTarget();
			break;
		case TankAIState::INVESTIGATE:
			InvestigateTargetLastLocation();
			break;
	}

	if ( m_shotCooldown > 0.f )
	{
		m_shotCooldown -= deltaSeconds;
	}

	TurnToward( m_goalOrientation, deltaSeconds );
	
	if ( m_isMoving )
	{
		m_velocity = Vec2::MakeFromPolarDegrees( m_goalOrientation ) * TANK_MAX_SPEED;
	}

	AdjustForWalls();
	
	Entity::Update( deltaSeconds );

	m_velocity.ClampLength( TANK_MAX_SPEED );
}


//-----------------------------------------------------------------------------------------------
void NpcTank::Render() const
{
	if ( IsDead() )
	{
		return;
	}

	std::vector<Vertex_PCU> vertexesCopy( m_vertexes );
	Vertex_PCU::TransformVertexArray( vertexesCopy, 1.f, m_orientationDegrees, m_position );

	g_renderer->BindTexture( m_tankBodyTexture );
	g_renderer->DrawVertexArray( vertexesCopy );

	RenderHealthBar();
}


//-----------------------------------------------------------------------------------------------
void NpcTank::DebugRender() const
{
	if ( m_isDead )
	{
		return;
	}

	// Draw chase range
	RaycastImpact impact1 = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees + TANK_CHASE_ANGLE_RANGE_DEGREES ), TANK_MAX_CHASE_RANGE );
	RaycastImpact impact2 = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees - TANK_CHASE_ANGLE_RANGE_DEGREES ), TANK_MAX_CHASE_RANGE );
	g_renderer->DrawLine2D( m_position, impact1.m_impactPosition, Rgba8::YELLOW, DEBUG_LINE_THICKNESS );
	g_renderer->DrawLine2D( m_position, impact2.m_impactPosition, Rgba8::YELLOW, DEBUG_LINE_THICKNESS );

	// Draw shot range
	impact1 = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees + TANK_SHOT_ANGLE_RANGE_DEGREES ), TANK_MAX_CHASE_RANGE );
	impact2 = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees - TANK_SHOT_ANGLE_RANGE_DEGREES ), TANK_MAX_CHASE_RANGE );
	g_renderer->DrawLine2D( m_position, impact1.m_impactPosition, Rgba8::GREEN, DEBUG_LINE_THICKNESS );
	g_renderer->DrawLine2D( m_position, impact2.m_impactPosition, Rgba8::GREEN, DEBUG_LINE_THICKNESS );

	// Draw whiskers
	g_renderer->DrawLine2D( m_position, m_middleWhisker.m_impactPosition, Rgba8::WHITE, DEBUG_LINE_THICKNESS );
	g_renderer->DrawLine2D( m_position, m_leftWhisker.m_impactPosition,	  Rgba8::WHITE, DEBUG_LINE_THICKNESS );
	g_renderer->DrawLine2D( m_position, m_rightWhisker.m_impactPosition,  Rgba8::WHITE, DEBUG_LINE_THICKNESS );

	Entity::DebugRender();
}


//-----------------------------------------------------------------------------------------------
void NpcTank::Die()
{
	m_isGarbage = true;
	m_isDead = true;

	g_audioSystem->PlaySound( m_diedSound );
	m_map->SpawnExplosion( m_position, 1.25f, 1.f );
}


//-----------------------------------------------------------------------------------------------
void NpcTank::TakeDamage( int damage )
{
	Entity::TakeDamage( damage );

	g_audioSystem->PlaySound( m_hitSound );
}


//-----------------------------------------------------------------------------------------------
void NpcTank::PopulateVertexes()
{
	m_tankBodyTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank3.png" );

	g_renderer->AppendVertsForAABB2D( m_vertexes, AABB2::ONE_BY_ONE, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
}


//-----------------------------------------------------------------------------------------------
void NpcTank::Wander( float deltaSeconds )
{
	Entity* target = m_map->GetPlayer();

	if ( CanSeeLivingTarget( target, TANK_MAX_CHASE_RANGE ) )
	{
		m_aiState = TankAIState::CHASE;
		return;
	}

	if ( m_wanderDirectionChangeCooldown <= 0.f )
	{
		m_goalOrientation = g_game->m_rng->RollRandomFloatInRange( 0.f, 360.f );
		m_wanderDirectionChangeCooldown = TANK_WANDER_DIRECTION_CHANGE_COOLDOWN_SECONDS;
	}
	else
	{
		m_wanderDirectionChangeCooldown -= deltaSeconds;
	}

	m_isMoving = true;
}


//-----------------------------------------------------------------------------------------------
void NpcTank::ChaseTarget()
{
	Entity* target = m_map->GetPlayer();

	if ( !CanSeeLivingTarget( target, TANK_MAX_CHASE_RANGE ) )
	{
		m_aiState = TankAIState::INVESTIGATE;
		return;
	}

	Vec2 displacementToTarget = target->GetPosition() - m_position;
	float angleDisplacementToTargetDegrees = displacementToTarget.GetOrientationDegrees();
	m_goalOrientation = angleDisplacementToTargetDegrees;

	if ( m_shotCooldown <= 0.f
		 && IsPointInForwardSector2D( target->GetPosition(), m_position, m_orientationDegrees, TANK_SHOT_ANGLE_RANGE_DEGREES * 2.f, TANK_MAX_CHASE_RANGE ) )
	{
		m_map->SpawnNewEntity( ENTITY_TYPE_EVIL_BULLET, m_faction, m_position + GetForwardVector() * m_physicsRadius, m_orientationDegrees );
		m_shotCooldown = TANK_SHOT_COOLDOWN;

		g_audioSystem->PlaySound( m_shootSound );
	}

	m_isMoving = IsPointInForwardSector2D( target->GetPosition(), m_position, m_orientationDegrees, TANK_CHASE_ANGLE_RANGE_DEGREES * 2.f, TANK_MAX_CHASE_RANGE );

	m_targetLastLocation = target->GetPosition();
}


//-----------------------------------------------------------------------------------------------
void NpcTank::InvestigateTargetLastLocation()
{
	Entity* target = m_map->GetPlayer();

	if ( CanSeeLivingTarget( target, TANK_MAX_CHASE_RANGE ) )
	{
		m_aiState = TankAIState::CHASE;
		return;
	}

	if (target->IsDead() 
		|| GetDistance2D( m_position, m_targetLastLocation ) < .1f )
	{
		m_aiState = TankAIState::WANDER;
		return;
	}

	Vec2 displacementToTarget = m_targetLastLocation - m_position;
	float angleDisplacementToTargetDegrees = displacementToTarget.GetOrientationDegrees();
	m_goalOrientation = angleDisplacementToTargetDegrees;

	m_isMoving = true;
}


//-----------------------------------------------------------------------------------------------
void NpcTank::AdjustForWalls()
{
	m_middleWhisker = m_map->Raycast( m_position, GetForwardVector(), TANK_MIDDLE_WHISKER_LENGTH );
	m_leftWhisker = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees + TANK_SIDE_WHISKER_ANGLE_DEGREES ), TANK_SIDE_WHISKER_LENGTH );
	m_rightWhisker = m_map->Raycast( m_position, Vec2::MakeFromPolarDegrees( m_orientationDegrees - TANK_SIDE_WHISKER_ANGLE_DEGREES ), TANK_SIDE_WHISKER_LENGTH );
	
	if ( m_middleWhisker.m_didImpact )
	{
		m_velocity -= m_velocity * TANK_WHISKER_SPEED_FACTOR;
	}
	
	if ( m_leftWhisker.m_didImpact )
	{
		m_goalOrientation -= TANK_WHISKER_TURN_DEGREES;
	}
	
	// Weight the right whisker a bit more to help with corners where both are touching
	if ( m_rightWhisker.m_didImpact )
	{
		m_goalOrientation += TANK_WHISKER_TURN_DEGREES + 1.f;
	}
}
