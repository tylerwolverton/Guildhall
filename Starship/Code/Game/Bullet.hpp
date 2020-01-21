#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class Bullet :
	public Entity
{
public:
	Bullet( Game* theGame, const Vec2& position, const Vec2& orientation );
	virtual ~Bullet() {}

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;
};

