#pragma once

struct Vec2;
class Rigidbody2D;
class Collider2D;
class DiscCollider2D;


class Physics2D
{
public:
	void BeginFrame();
	void Update();
	void EndFrame();

	// factory style create/destroy
	Rigidbody2D* CreateRigidbody();
	void DestroyRigidbody( Rigidbody2D* rb );

	DiscCollider2D* CreateDiscCollider( const Vec2& localPosition, float center );
	void DestroyCollider( Collider2D* collider );

private:
	// vector of rigidbodies

	//vector of colliders

};
