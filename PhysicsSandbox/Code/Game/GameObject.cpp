#include "Game/GameObject.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
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


//-----------------------------------------------------------------------------------------------
void GameObject::SetRigidbody( Rigidbody2D* rigidbody )
{
	m_rigidbody = rigidbody;
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetCollider( Collider2D* collider )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->TakeCollider( collider );
}


//-----------------------------------------------------------------------------------------------
Vec2 GameObject::GetVelocity() const
{
	if ( m_rigidbody == nullptr )
	{
		return Vec2::ZERO;
	}

	return m_rigidbody->GetVelocity();
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetVelocity( const Vec2& velocity )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetVelocity( velocity );
}


//-----------------------------------------------------------------------------------------------
Vec2 GameObject::GetVerletVelocity() const
{
	if ( m_rigidbody == nullptr )
	{
		return Vec2::ZERO;
	}

	return m_rigidbody->GetVerletVelocity();
}


//-----------------------------------------------------------------------------------------------
Vec2 GameObject::GetPosition()
{
	if ( m_rigidbody == nullptr )
	{
		return Vec2::ZERO;
	}

	return m_rigidbody->GetPosition();
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetPosition( const Vec2& position )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetPosition( position );
}


//-----------------------------------------------------------------------------------------------
bool GameObject::IsPhysicsEnabled() const
{
	if ( m_rigidbody == nullptr )
	{
		return false;
	}

	return m_rigidbody->IsEnabled();
}


//-----------------------------------------------------------------------------------------------
void GameObject::EnablePhysics()
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->Enable();
}


//-----------------------------------------------------------------------------------------------
void GameObject::DisablePhysics()
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->Disable();
}


//-----------------------------------------------------------------------------------------------
eSimulationMode GameObject::GetSimulationMode()
{
	if ( m_rigidbody == nullptr )
	{
		return SIMULATION_MODE_DYNAMIC;
	}

	return m_rigidbody->GetSimulationMode();
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetSimulationMode( eSimulationMode mode )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetSimulationMode( mode );
}


//-----------------------------------------------------------------------------------------------
uint GameObject::GetLayer() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0;
	}

	return m_rigidbody->GetLayer();
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetLayer( uint layer )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetLayer( layer );
}


//-----------------------------------------------------------------------------------------------
bool GameObject::Contains( const Vec2& point ) const
{
	if ( m_rigidbody == nullptr )
	{
		return false;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return false;
	}

	return collider->Contains( point );
}


//-----------------------------------------------------------------------------------------------
bool GameObject::Intersects( const GameObject* otherGameObject ) const
{
	if ( m_rigidbody == nullptr )
	{
		return false;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return false;
	}

	if ( otherGameObject->m_rigidbody == nullptr )
	{
		return false;
	}

	Collider2D* otherCollider = otherGameObject->m_rigidbody->GetCollider();
	if ( otherCollider == nullptr )
	{
		return false;
	}

	return collider->Intersects( otherCollider );
}


//-----------------------------------------------------------------------------------------------
unsigned int GameObject::CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const
{
	if ( m_rigidbody == nullptr )
	{
		return 0;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return 0;
	}

	return collider->CheckIfOutsideScreen( screenBounds, checkForCompletelyOffScreen );
}


//-----------------------------------------------------------------------------------------------
const AABB2 GameObject::GetBoundingBox() const
{
	if ( m_rigidbody == nullptr )
	{
		return AABB2( Vec2::ZERO, Vec2::ZERO );
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return AABB2( Vec2::ZERO, Vec2::ZERO );
	}

	return collider->GetWorldBounds();
}


//-----------------------------------------------------------------------------------------------
void GameObject::RotateDegrees( float deltaDegrees )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->RotateDegrees( deltaDegrees );
}


//-----------------------------------------------------------------------------------------------
void GameObject::ChangeBounciness( float deltaBounciness )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return;
	}

	collider->m_material.m_bounciness += deltaBounciness;
	collider->m_material.m_bounciness = ClampMinMax( collider->m_material.m_bounciness, 0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
void GameObject::ChangeFriction( float deltaFriction )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return;
	}

	collider->ChangeFriction( deltaFriction );
}


//-----------------------------------------------------------------------------------------------
void GameObject::ChangeMass( float deltaMass )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->ChangeMass( deltaMass );
}


//-----------------------------------------------------------------------------------------------
void GameObject::ChangeDrag( float deltaDrag )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->ChangeDrag( deltaDrag );
}


//-----------------------------------------------------------------------------------------------
void GameObject::ChangeAngularVelocity( float deltaAnglularVelocity )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->ChangeAngularVelocity( deltaAnglularVelocity );
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetRotationDegrees( float newDegrees )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetRotationDegrees( newDegrees );
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetAngularVelocity( float newAngularVelocity )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetAngularVelocity( newAngularVelocity );
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetBounciness() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return 0.f;
	}

	return collider->m_material.m_bounciness;
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetFriction() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	Collider2D* collider = m_rigidbody->GetCollider();
	if ( collider == nullptr )
	{
		return 0.f;
	}

	return collider->m_material.m_friction;
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetMass() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	return m_rigidbody->GetMass();
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetMomentOfInertia() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	return m_rigidbody->GetMomentOfInertia();
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetDrag() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	return m_rigidbody->GetDrag();
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetRotationDegrees() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	return m_rigidbody->GetOrientationDegrees();
}


//-----------------------------------------------------------------------------------------------
float GameObject::GetAngularVelocity() const
{
	if ( m_rigidbody == nullptr )
	{
		return 0.f;
	}

	return m_rigidbody->GetAngularVelocity();
}
