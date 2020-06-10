#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class ActorDefinition;
class SpriteAnimDefinition;


//-----------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( const Vec2& position, ActorDefinition* actorDef );
	~Actor();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

	bool IsPlayer() const												{ return m_isPlayer; }

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromGamepad( float deltaSeconds );
	void UpdateAnimation();

	void MoveToTargetLocation();

protected:
	ActorDefinition*		m_actorDef = nullptr;
	int						m_controllerID = -1;
	bool					m_isPlayer = false;
	float					m_cumulativeTime = 0.f;
	
	float					m_wanderDirectionChangeCooldown = 0.f;
	Vec2					m_moveTargetLocation = Vec2::ZERO;
	float					m_speed = 0.f;

	SpriteAnimDefinition*	m_curAnimDef = nullptr;
};
