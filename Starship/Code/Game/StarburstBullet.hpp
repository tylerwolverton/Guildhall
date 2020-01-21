#pragma once
#include "Game/Bullet.hpp"


//-----------------------------------------------------------------------------------------------
class StarburstBullet :
	public Bullet
{
public:
	StarburstBullet(Game* theGame, const Vec2& position, const Vec2& orientation);
	virtual ~StarburstBullet() {}

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
};

