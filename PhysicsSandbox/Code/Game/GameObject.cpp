#include "Game/GameObject.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
GameObject::~GameObject()
{
	m_rigidbody->Destroy();
	m_rigidbody = nullptr;
}


//-----------------------------------------------------------------------------------------------
void GameObject::DebugRender()
{
	if ( m_rigidbody != nullptr )
	{
		Rgba8 fillColor = m_fillColor;
		fillColor.a = (unsigned char)( (float)fillColor.a * .5f );
		m_rigidbody->DebugRender( g_renderer, m_borderColor, fillColor );
	}
}
