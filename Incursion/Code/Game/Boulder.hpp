#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class Texture;
class SpriteSheet;


//-----------------------------------------------------------------------------------------------
class Boulder :
	public Entity
{
public:
	Boulder( Map* map, EntityFaction faction, const Vec2& position );
	~Boulder();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

private:
	void PopulateVertexes();

private:
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_texture = nullptr;
	SpriteSheet*			m_extrasSpriteSheet = nullptr;
};

