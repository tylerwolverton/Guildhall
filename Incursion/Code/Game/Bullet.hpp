#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class Texture;


//-----------------------------------------------------------------------------------------------
class Bullet :
	public Entity
{
public:
	Bullet( Map* map, EntityType type, EntityFaction faction, const Vec2& position, float orientationDegrees );
	~Bullet();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

private:
	void PopulateVertexes();

private:
	// Visual
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_texture = nullptr;
};
