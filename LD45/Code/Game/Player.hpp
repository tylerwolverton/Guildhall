#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
class Player : public Entity
{
public:
	Player( Game* theGame, const Vec2& position );
	virtual ~Player() {}

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void Collide( const Entity* otherEntity ) override;
	virtual void DebugRender() const override;

	bool IsInvincible()									{ return m_isInvincible; }

	void AddVertexes( std::vector<Vertex_PCU> vertexes );

	void CaptureEntity( Entity* capturedEntity );

	void TakeDamage();

	void AddImpulse( float magnitude, const Vec2& direction);

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromGamepad( float deltaSeconds );
	void Respawn();
	void CheckIfOutOfBounds();

	void FireProjectile();

	/*std::vector<Vertex_PCU> m_playerVertexes;
	std::vector<Vertex_PCU> m_playerVertexesCopy;*/

	std::vector<Entity*>  m_capturedEntities;

	int m_controllerID = 0;

	float m_invincibilityTimer = 0.f;
	bool m_isInvincible = false;

	std::vector<float> m_physicsRadiusStack;
};