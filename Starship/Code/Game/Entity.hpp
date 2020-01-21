#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
class Game;


//-----------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity( Game* theGame, const Vec2& position );
	virtual ~Entity() {}

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	virtual void DebugRender() const;

	const Vec2	GetForwardVector() const;
	const Vec2	GetPosition() const			{ return m_position; };
	const float GetPhysicsRadius() const	{ return m_physicsRadius; };

	bool IsDead() const			{ return m_isDead; }
	bool IsGarbage() const		{ return m_isGarbage; }
	bool IsOffScreen() const;

	void TakeDamage( int damage );

protected:
	Vec2  m_position = Vec2( 0.f, 0.f );			// the Entity’s 2D(x, y) Cartesian origin / center location, in world space
	Vec2  m_velocity = Vec2( 0.f, 0.f );			// the Entity’s linear 2D( x, y ) velocity, in world units per second
	Vec2  m_linearAcceleration = Vec2( 0.f, 0.f );	// the Entity’s signed linear acceleration per second per second
	float m_orientationDegrees = 0.f;				// the Entity’s forward - facing direction, as an angle in degrees
	float m_angularVelocity = 0.f;					// the Entity’s signed angular velocity( spin rate ), in degrees per second
	float m_physicsRadius = 10.f;					// the Entity’s( inner, conservative ) disc - radius for all physics purposes
	float m_cosmeticRadius = 20.f;					// the Entity’s( outer, liberal ) disc - radius that fully encompasses it
	int   m_health = 1;								// how many “hits” the entity can sustain before dying
	bool  m_isDead = false;							// whether the Entity is “dead” in the game; affects entity and game logic
	bool  m_isGarbage = false;						// whether the Entity should be deleted at the end of Game::Update()
	Game* m_game = nullptr;							// a pointer back to the Game instance
	Rgba8 m_mainColor = Rgba8(255, 255, 255);
	int   m_numDebrisPieces = 0;
	float m_hitFlashTimer = 0.f;
};
