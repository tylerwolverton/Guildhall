#pragma once
//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Rgba8;
class Physics2D;
class Rigidbody2D;
class Collider2D;


//-----------------------------------------------------------------------------------------------
enum eCollider2DType
{
	COLLIDER2D_DISC,
};


//-----------------------------------------------------------------------------------------------
// Interface for all Collider objects used with our Physics system
class Collider2D
{
	friend class Physics2D;

public: // Interface 
	// cache off the world shape representation of this object
	// taking into account the owning rigidbody (if no owner, local is world)
	virtual void UpdateWorldShape() const = 0;

	// queries 
	virtual const Vec2 GetClosestPoint( const Vec2& pos ) const = 0;
	virtual bool Contains( const Vec2& pos ) const = 0;
	virtual bool Intersects( const Collider2D* other ) const = 0;


	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) = 0;

protected:
	// 
	virtual ~Collider2D() = 0; // private - make sure this is virtual so correct deconstructor gets called

public: // any helpers you want to add
   // ...

public:
	eCollider2DType m_type;                 // keep track of the type - will help with collision later
	Physics2D* m_system;                   // system who created our d
	Rigidbody2D* m_rigidbody = nullptr;    // owning rigidbody, used for calculating world shape
};
