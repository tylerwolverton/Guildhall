#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/EntityDefinition.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class SpriteDefinition;
class SpriteAnimDefinition;
class SpriteAnimSetDefinition;


//-----------------------------------------------------------------------------------------------
class Entity
{
	friend class Map;
	
public:
	Entity( const Vec2& position, EntityDefinition* entityDef );
	virtual ~Entity();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	virtual void DebugRender() const;

	const Vec2	 GetForwardVector() const;
	const Vec2	 GetPosition() const								{ return m_position; };
	void		 SetPosition( const Vec2& newPos )					{ m_position = newPos; }
	const float  GetPhysicsRadius() const							{ return m_entityDef->m_physicsRadius; };
	const float  GetTopOfSprite() const								{ return m_entityDef->m_localDrawBounds.maxs.y; };
	std::string  GetName() const									{ return m_entityDef->m_name; };
	std::string  GetType() const									{ return m_entityDef->m_type; };
	Texture*	 GetTexture() const									{ return m_texture; };
	int			 GetDrawOrder() const								{ return m_entityDef->m_drawOrder; };
				 
	bool		 IsDead() const										{ return m_isDead; }
	bool		 IsGarbage() const									{ return m_isGarbage; }

	void		 SetIsInPlayerInventory( bool isInPlayerInventory ) { m_isInPlayerInventory = isInPlayerInventory; }
	bool		 IsInPlayerInventory() const						{ return m_isInPlayerInventory; }

	void		 TakeDamage( int damage );
	void		 ApplyFriction();

	// Item functions
	SpriteDefinition* GetSpriteDef() const;

	void SetTexture( Texture* texture )								{ m_texture = texture; }
	void Open()														{ m_isOpen = true; }
	void Close()													{ m_isOpen = false; }
	bool IsOpen() const												{ return m_isOpen; }
	void AddVerbState( eVerbState verbState, NamedProperties* properties );
	void RemoveVerbState( eVerbState verbState );

	// Events
	void HandleVerbAction( eVerbState verbState );

protected:
	void PopulateVertexes();
	void UpdateAnimation();

protected:
	// Game state
	EntityDefinition*		m_entityDef = nullptr;
	int						m_curHealth = 1;								// how much health is currently remaining on entity
	bool					m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool					m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool					m_isInPlayerInventory = false;
	bool					m_isOpen = false;

	// Physics
	Vec2					m_position = Vec2( 0.f, 0.f );					// the Entity’s 2D(x, y) Cartesian origin / center location, in world space
	Vec2					m_velocity = Vec2( 0.f, 0.f );					// the Entity’s linear 2D( x, y ) velocity, in world units per second
	Vec2					m_linearAcceleration = Vec2( 0.f, 0.f );		// the Entity’s signed linear acceleration per second per second
	float					m_orientationDegrees = 0.f;						// the Entity’s forward - facing direction, as an angle in degrees
	float					m_angularVelocity = 0.f;						// the Entity’s signed angular velocity( spin rate ), in degrees per second
	
	// Visual
	std::vector<Vertex_PCU> m_vertexes;
	float					m_cumulativeTime = 0.f;
	SpriteAnimDefinition*	m_curAnimDef = nullptr;
	Texture*				m_texture = nullptr;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;
