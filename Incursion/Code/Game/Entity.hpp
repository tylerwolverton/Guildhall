#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;


//-----------------------------------------------------------------------------------------------
enum EntityFaction
{
	ENTITY_FACTION_INVALID = -1,

	ENTITY_FACTION_NEUTRAL,
	ENTITY_FACTION_GOOD,
	ENTITY_FACTION_EVIL,

	NUM_ENTITY_FACTIONS
};


//-----------------------------------------------------------------------------------------------
enum EntityType
{
	ENTITY_TYPE_INVALID = -1,

	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_NPC_TURRET,
	ENTITY_TYPE_NPC_TANK,
	ENTITY_TYPE_BOULDER,
	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_EXPLOSION,

	NUM_ENTITY_TYPES
};


//-----------------------------------------------------------------------------------------------
class Entity
{
	friend class Map;
	
public:
	Entity( Map* map, EntityType type, EntityFaction faction, const Vec2& position );
	virtual ~Entity() {}

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	virtual void DebugRender() const;

	const Vec2			GetForwardVector() const;
	const Vec2			GetPosition() const							{ return m_position; };
	const float			GetPhysicsRadius() const					{ return m_physicsRadius; };
	const EntityType	GetType() const								{ return m_type; };
	const EntityFaction GetFaction() const							{ return m_faction; };

	bool				IsDead() const								{ return m_isDead; }
	bool				IsGarbage() const							{ return m_isGarbage; }
	bool				IsOffScreen() const;
	bool				PushesEntities() const						{ return m_pushesEntities; }
	bool				IsPushedByWalls() const						{ return m_isPushedByWalls; }
	bool				IsPushedByEntities() const					{ return m_isPushedByEntities; }
	bool				IsHitByBullets() const						{ return m_isHitByBullets; }

	virtual void		TakeDamage( int damage );

	void				TurnToward( float goalOrientationDegrees, float deltaSeconds );

protected:
	void				ApplyFriction();
	void				UpdateHitFlash( float deltaSeconds );
	bool				CanSeeTarget( const Entity& target, float maxDistance ) const;
	bool				CanSeeLivingTarget( const Entity* target, float maxDistance ) const;
	void				RenderHealthBar() const;

protected:
	// Game state
	Map*			m_map = nullptr;
	EntityType		m_type = ENTITY_TYPE_INVALID;
	EntityFaction	m_faction = ENTITY_FACTION_INVALID;
	int				m_health = 1;									// how many “hits” the entity has left before dying
	int				m_maxHealth = 1;								// how many “hits” the entity can sustain before dying
	bool			m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool			m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool			m_pushesEntities = false;
	bool			m_isPushedByWalls = false;
	bool			m_isPushedByEntities = false;
	bool			m_isHitByBullets = false;

	// Physics
	Vec2			m_position = Vec2( 0.f, 0.f );					// the Entity’s 2D(x, y) Cartesian origin / center location, in world space
	Vec2			m_velocity = Vec2( 0.f, 0.f );					// the Entity’s linear 2D( x, y ) velocity, in world units per second
	Vec2			m_linearAcceleration = Vec2( 0.f, 0.f );		// the Entity’s signed linear acceleration per second per second
	float			m_orientationDegrees = 0.f;						// the Entity’s forward - facing direction, as an angle in degrees
	float			m_angularVelocity = 0.f;						// the Entity’s signed angular velocity( spin rate ), in degrees per second
	float			m_maxTurnSpeed = 0.f;
	float			m_physicsRadius = 10.f;							// the Entity’s( inner, conservative ) disc - radius for all physics purposes
	float			m_movementSpeedFraction = 1.f;

	// Visual
	float			m_cosmeticRadius = 20.f;						// the Entity’s( outer, liberal ) disc - radius that fully encompasses it
	Rgba8			m_mainColor = Rgba8::WHITE;						// most prevalent color; used for debris
	float			m_hitFlashTimer = 0.f;							// how long to flash when damaged
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;
