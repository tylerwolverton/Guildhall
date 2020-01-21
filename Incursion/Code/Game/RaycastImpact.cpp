#include "Game/RaycastImpact.hpp"


//-----------------------------------------------------------------------------------------------
RaycastImpact::RaycastImpact( bool didImpact, const Vec2& impactPosition )
	: m_didImpact( didImpact )
	, m_impactPosition( impactPosition )
{
}
