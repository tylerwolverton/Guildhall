#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class Beetle :
	public Entity
{
public:
	Beetle(Game* theGame, const Vec2& position);
	virtual ~Beetle();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
};

