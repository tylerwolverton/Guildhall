#pragma once
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec3;
class GPUMesh;
class Material;
class Rigidbody2D;
class InteractableSwitch;


//-----------------------------------------------------------------------------------------------
class GameObject
{
public:
	virtual void Update();
	void UpdateTransform();
	void Render() const;

	void SetRigidbody( Rigidbody2D* rigidbody );
	void SetMesh( GPUMesh* mesh )										{ m_mesh = mesh; }
	void SetMaterial( Material* material )								{ m_material = material; }
	void SetTransform( const Transform& transform )						{ m_transform = transform; }
	void SetOrientation( const Vec3& orientation );
	void SetSelectedSwitch( InteractableSwitch* gameObject )			{ m_selectedSwitch = gameObject; }

	void Translate( const Vec3& translation );
	void ApplyImpulseAt( const Vec3& impulse, const Vec3& position = Vec3::ZERO );

	Vec3 GetPosition() const;
	Vec3 GetForwardVector() const;
	InteractableSwitch* GetSelectedSwitch() const						{ return m_selectedSwitch; }
;
protected:
	Transform m_transform;
	GPUMesh* m_mesh = nullptr;
	Rigidbody2D* m_rigidbody = nullptr;
	Material* m_material = nullptr;

	InteractableSwitch* m_selectedSwitch = nullptr;
};