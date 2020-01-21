#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Entity.hpp"
#include "Game/RaycastImpact.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;


//-----------------------------------------------------------------------------------------------
enum class TurretAIState
{
	SEARCH,
	ATTACK
};


//-----------------------------------------------------------------------------------------------
class NpcTurret :
	public Entity
{
public:
	NpcTurret( Map* map, EntityFaction faction, const Vec2& position );
	~NpcTurret();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void DebugRender() const;
	virtual void Die();
	virtual void TakeDamage( int damage );

private:
	void PopulateVertexes();

	void Search( float deltaSeconds );
	void Attack( float deltaSeconds );
	
private:
	// Game state
	TurretAIState		m_aiState = TurretAIState::SEARCH;
	float				m_shotCooldown = 0.f;
	bool				m_hasSeenTarget = false;
	float				m_angleToTargetLastLocationDegrees = 0.f;
	
	// Physics
	float			m_goalOrientation = 0.f;
	RaycastImpact	m_lastRaycastImpact = RaycastImpact( false, Vec2( 0.f, 0.f ) );

	// Visual
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_turretBase = nullptr;
	Texture*				m_turretTop = nullptr;

	// Audio
	SoundID m_diedSound;
	SoundID m_hitSound;
	SoundID m_shootSound;
};
