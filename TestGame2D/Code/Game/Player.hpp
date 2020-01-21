#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;


//-----------------------------------------------------------------------------------------------
class Player :
	public Entity
{
	friend class Map;

public:
	Player( EntityFaction faction, const Vec2& position );
	~Player();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

private:
	void PopulateVertexes();

	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromGamepad( float deltaSeconds );

private:
	int m_controllerID = 0;

	std::vector<Vertex_PCU> m_vertexes;
	Texture* m_tankBodyTexture = nullptr;
};

