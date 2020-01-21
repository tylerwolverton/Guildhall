#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

struct Rgba8;

class Enemy : public Entity
{

public:
	Enemy( Game* game, Vec2 position, const Rgba8& color );
	Enemy( Game* game, Vec2 position, std::vector<Vertex_PCU> vertexes, const Rgba8& color );
	~Enemy();
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void Collide( const Entity* otherEntity ) override;
	virtual void DebugRender() const override;

	virtual void BecomeCaptured() override;

	void CheckIfOutOfBounds();

	void CheckIfStopped();
private:
	Vec2 m_moveDirection = Vec2( 0.f, 0.f );
	float m_moveTimer = 0.f;

	/*std::vector<Vertex_PCU> m_vertexes;
	std::vector<Vertex_PCU> m_vertexesCopy;*/

};