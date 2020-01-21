#pragma once
#include "Game/Entity.hpp"

class Wasp :
	public Entity
{
public:
	Wasp( Game* theGame, const Vec2& position );
	virtual ~Wasp();

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
};

