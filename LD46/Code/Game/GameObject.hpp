#pragma once
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec3;
class GPUMesh;
class Material;
class Rigidbody2D;


//-----------------------------------------------------------------------------------------------
class GameObject
{
public:
	void Update();
	void UpdateTransform();
	void Render() const;

	void SetRigidbody( Rigidbody2D* rigidbody );
	void SetMesh( GPUMesh* mesh )										{ m_mesh = mesh; }
	void SetMaterial( Material* material )								{ m_material = material; }
	void SetTransform( const Transform& transform )						{ m_transform = transform; }

	Vec3 GetPosition();

private:
	Transform m_transform;
	GPUMesh* m_mesh = nullptr;
	Rigidbody2D* m_rigidbody = nullptr;
	Material* m_material = nullptr;
};