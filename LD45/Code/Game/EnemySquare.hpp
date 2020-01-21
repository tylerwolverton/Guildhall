#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

struct Rgba8;

class EnemySquare : public Entity
{

public:
	EnemySquare( Game* game, float sizeModifier, Vec2 position, const Rgba8& color );
	~EnemySquare();
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void Collide( const Entity* otherEntity ) override;
	virtual void DebugRender() const override;

	void CheckIfOutOfBounds();

	virtual void AttachAttacker( Entity* attacker ) override;
	void AddVertexes( std::vector<Vertex_PCU> vertexes );

private:
	Vec2 m_moveDirection = Vec2( 0.f, 0.f );
	float m_moveTimer = 0.f;
	float m_speed = ENEMY_SPEED;

	std::vector<Entity*> m_attackers;

};