#include "Game/GameObject.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"


//-----------------------------------------------------------------------------------------------
GameObject::~GameObject()
{
	m_rigidbody->Destroy();
	m_rigidbody = nullptr;
}
