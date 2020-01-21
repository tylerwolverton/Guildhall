#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class Debris :
	public Entity
{
public:
	Debris(Game* theGame, const Vec2& position, const Vec2& velocity, const Rgba8& color);
	virtual ~Debris();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	void PopulateVertexes();

private:
	float		m_ageSeconds = 0.0f;
	Rgba8		m_color = Rgba8( 0, 0, 0 );
	Vertex_PCU	m_localRenderingVertexes[NUM_DEBRIS_VERTS];
};

