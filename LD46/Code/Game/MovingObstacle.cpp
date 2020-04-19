#include "Game/MovingObstacle.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"


//-----------------------------------------------------------------------------------------------
MovingObstacle::MovingObstacle( float duration, const Vec3& velocity )
	: m_velocity( velocity )
{
	m_moveTimer = Timer();
	m_moveTimer.SetSeconds( (double)duration );
}


//-----------------------------------------------------------------------------------------------
void MovingObstacle::Update()
{
	if ( m_moveTimer.CheckAndReset() )
	{
		m_velocity *= -1.f;
		m_rigidbody->SetVelocity( Vec2( m_velocity.x, m_velocity.z ) );
	}
}
