#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class SpriteAnimDefinition;


//-----------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( const EntityDefinition& entityDef );
	~Actor();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromGamepad( float deltaSeconds );
	//void UpdateAnimation();

protected:
	int						m_controllerID = -1;
	bool					m_isPlayer = false;
};
