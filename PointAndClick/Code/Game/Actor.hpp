#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Texture;
class EntityDefinition;
class SpriteAnimDefinition;



//-----------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( const Vec2& position, EntityDefinition* entityDef );
	~Actor();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

	bool IsPlayer() const												{ return m_isPlayer; }
	bool IsExecutingAction() const										{ return m_isExecutingAction; }
	void ExecuteAction( const std::string& nounName );
	void StopExecutingAction();

	// Only ever called by Game
	void SetPlayerVerbState( eVerbState verbState )						{ m_curVerbState = verbState; }
	eVerbState GetPlayerVerbState() const								{ return m_curVerbState; }
	void SetMoveTargetLocation( const Vec2& moveTarget )				{ m_moveTargetLocation = moveTarget; }
	void SetMap( Map* map )												{ m_map = map; }

private:
	void MoveToTargetLocation();

protected:
	Map*					m_map = nullptr;
	int						m_controllerID = -1;
	bool					m_isPlayer = false;
	bool					m_isExecutingAction = false;
	std::string				m_targetNoun;
	bool					m_moveStarted = false;
	float					m_cumulativeTime = 0.f;
	
	float					m_wanderDirectionChangeCooldown = 0.f;
	Vec2					m_moveTargetLocation = Vec2::ZERO;
	float					m_speed = 0.f;

	SpriteAnimDefinition*	m_curAnimDef = nullptr;

	eVerbState m_curVerbState = eVerbState::NONE;
};
