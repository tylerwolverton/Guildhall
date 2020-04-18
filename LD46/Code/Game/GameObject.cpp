#include "Game/GameObject.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Material.hpp"


//-----------------------------------------------------------------------------------------------
void GameObject::Update()
{
	/*if ( m_rigidbody != nullptr )
	{
		m_transform.SetPosition( Vec3( m_rigidbody->GetPosition().x, 0.f, m_rigidbody->GetPosition().y ) );
	}*/
}


//-----------------------------------------------------------------------------------------------
void GameObject::UpdateTransform()
{
	if ( m_rigidbody != nullptr )
	{
		m_transform.SetPosition( Vec3( m_rigidbody->GetPosition().x, 0.f, m_rigidbody->GetPosition().y ) );
	}
}


//-----------------------------------------------------------------------------------------------
void GameObject::Render() const
{
	float specularFactor = 0.f;
	float specularPower = 32.f;
	if ( m_material != nullptr )
	{
		specularFactor = m_material->GetSpecularFactor();
		specularPower = m_material->GetSpecularPower();

		m_material->Bind();
	}

	if ( m_mesh != nullptr )
	{
		Mat44 model = m_transform.GetAsMatrix();
		g_renderer->SetModelData( model, Rgba8::WHITE, specularFactor, specularPower );
		g_renderer->DrawMesh( m_mesh );
	}
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetRigidbody( Rigidbody2D* rigidbody )
{
	m_rigidbody = rigidbody;

	m_transform.SetPosition( Vec3( m_rigidbody->GetPosition().x, 0.f, m_rigidbody->GetPosition().y ) );
}


//-----------------------------------------------------------------------------------------------
void GameObject::Translate( const Vec3& translation )
{
	m_transform.Translate( translation );
	
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->Translate2D( Vec2( translation.x, translation.z ) );
	}
}


//-----------------------------------------------------------------------------------------------
void GameObject::SetOrientation( const Vec3& orientation )
{
	m_transform.SetOrientation( orientation );
}


//-----------------------------------------------------------------------------------------------
Vec3 GameObject::GetPosition() const
{
	return m_transform.GetPosition();
}


//-----------------------------------------------------------------------------------------------
Vec3 GameObject::GetForwardVector() const
{
	return m_transform.GetForwardVector();
}
