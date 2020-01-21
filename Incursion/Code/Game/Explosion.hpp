#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


//-----------------------------------------------------------------------------------------------
class Texture;
class SpriteAnimDefinition;
class SpriteSheet;


//-----------------------------------------------------------------------------------------------
class Explosion :
	public Entity
{
public:
	Explosion( Map* map, const Vec2& position, float radius, float durationSeconds );
	virtual ~Explosion();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	virtual void DebugRender() const;

private:
	void PopulateVertexes();

private:
	// Game state
	float m_radius = 0.f;
	float m_durationSeconds = 0.f;
	float m_cumulativeTime = 0.f;

	// Visual
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_texture = nullptr;
	SpriteAnimDefinition*	m_spriteAnimDef = nullptr;
	SpriteSheet*			m_spriteSheet = nullptr;
};
