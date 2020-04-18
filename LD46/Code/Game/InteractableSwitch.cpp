#include "Game/InteractableSwitch.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/DebugRender.hpp"


//-----------------------------------------------------------------------------------------------
InteractableSwitch::InteractableSwitch( GameObject* player, float radius )
	: m_player( player )
	, m_radius( radius )
{
}


//-----------------------------------------------------------------------------------------------
void InteractableSwitch::Update()
{
	// Check if in range
	Vec2 playerPosition2D( m_player->GetPosition().x, m_player->GetPosition().z );
	Vec2 position2D( m_transform.GetPosition().x, m_transform.GetPosition().z );

	if ( !DoDiscsOverlap( playerPosition2D, .25f, position2D, m_radius ) )
	{
		return;
	}

	// Check if player is facing switch
	Vec3 playerDirection( m_player->GetForwardVector() );
	Vec3 direction( m_transform.GetForwardVector() );

	if ( DotProduct3D( playerDirection, direction ) >= 0.f )
	{
		return;
	}

	DebugAddWorldBillboardText( m_transform.GetPosition(), Vec2( 0.f, 0.f ), Rgba8::WHITE, Rgba8::WHITE, 0.f, DEBUG_RENDER_USE_DEPTH, m_helpText.c_str() );
}
