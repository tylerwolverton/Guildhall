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
	Actor( const EntityDefinition& entityDef, Map* map );
	~Actor();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;

	void SetAsPlayer();

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromGamepad( float deltaSeconds );
	//void UpdateAnimation();

protected:
	int	 m_controllerID = -1;
};
