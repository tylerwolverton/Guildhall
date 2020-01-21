#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Entity.hpp"
#include "Game/RaycastImpact.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;


//-----------------------------------------------------------------------------------------------
enum class TankAIState
{
	WANDER,
	CHASE,
	INVESTIGATE
};


//-----------------------------------------------------------------------------------------------
class NpcTank :
	public Entity
{
public:
	NpcTank( Map* map, EntityFaction faction, const Vec2& position );
	~NpcTank();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void DebugRender() const;
	virtual void Die();
	virtual void TakeDamage( int damage );

private:
	void PopulateVertexes();
	void Wander( float deltaSeconds );
	void ChaseTarget();
	void InvestigateTargetLastLocation();
	void AdjustForWalls();

private:
	// Game state
	TankAIState		m_aiState = TankAIState::WANDER;
	float			m_wanderDirectionChangeCooldown = 0.f;
	float			m_shotCooldown = 0.f;
	Vec2			m_targetLastLocation = Vec2::ZERO;

	// Physics
	float			m_goalOrientation = 0.f;
	bool			m_isMoving = false;
	RaycastImpact	m_middleWhisker;
	RaycastImpact	m_leftWhisker;
	RaycastImpact	m_rightWhisker;

	// Visual
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_tankBodyTexture = nullptr;

	// Audio
	SoundID m_diedSound;
	SoundID m_hitSound;
	SoundID m_shootSound;
};
