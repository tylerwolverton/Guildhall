#include "Game/InteractableSwitch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
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
	if ( !m_isEnabled )
	{
		return;
	}

	// Check if in range
	Vec2 playerPosition2D( m_player->GetPosition().x, m_player->GetPosition().z );
	Vec2 position2D( m_transform.GetPosition().x, m_transform.GetPosition().z );

	if ( !DoDiscsOverlap( playerPosition2D, .5f, position2D, m_radius ) )
	{
		m_player->SetSelectedSwitch( nullptr );
		return;
	}

	// Check if player is facing switch
	Vec3 playerDirection( m_player->GetForwardVector() );
	Vec3 direction( m_transform.GetForwardVector() );

	if ( DotProduct3D( playerDirection, direction ) >= 0.f )
	{
		m_player->SetSelectedSwitch( nullptr );
		return;
	}

	DebugAddWorldBillboardText( m_transform.GetPosition() + direction * .1f, Vec2( .5f, .5f ), .01f, Rgba8::WHITE, Rgba8::WHITE, 0.f, DEBUG_RENDER_USE_DEPTH, m_helpText.c_str() );
	m_player->SetSelectedSwitch( this );
}


//-----------------------------------------------------------------------------------------------
void InteractableSwitch::Enable()
{
	m_isEnabled = true;
}


//-----------------------------------------------------------------------------------------------
void InteractableSwitch::Activate()
{
	g_eventSystem->FireEvent( "light_switch_activated" );

	m_player->SetSelectedSwitch( nullptr );
	m_isEnabled = false;
}
