#pragma once
#include "Game/GameObject.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"


class MovingObstacle : public GameObject
{
public:
	MovingObstacle( float duration, const Vec3& velocity );

	virtual void Update() override;

private:
	Vec3 m_velocity;
	Timer m_moveTimer;
};