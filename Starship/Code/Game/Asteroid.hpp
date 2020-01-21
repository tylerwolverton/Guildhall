#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class Asteroid :
	public Entity
{
public:
	Asteroid(Game* theGame, const Vec2& position);
	virtual ~Asteroid() {}

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
	
private:
	void PopulateVertexes();

private:
	Vertex_PCU m_localRenderingVertexes[NUM_ASTEROID_VERTS];
};

