#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Rigidbody2D;
class SpriteAnimDefinition;
class ZephyrScript;


//-----------------------------------------------------------------------------------------------
enum class eFaction
{
	GOOD,
	EVIL,
	NEUTRAL
};


//-----------------------------------------------------------------------------------------------
class Entity
{
	friend class Map;
	friend class TileMap;
	
public:
	Entity( const EntityDefinition& entityDef, Map* map );
	virtual ~Entity();

	virtual void	Update( float deltaSeconds );
	virtual void	Render() const;
	virtual void	Die();
	virtual void	DebugRender() const;

	virtual void	Load();
	virtual void	Unload();

	void			ChangeZephyrScriptState( const std::string& targetState );
	void			UnloadZephyrScript();
	void			ReloadZephyrScript();
	void			InitializeScriptValues( const ZephyrValueMap& initialValues );

	const Vec2		GetForwardVector() const;
	const Vec2		GetPosition() const;
	void			SetPosition( const Vec2& position );
	void			SetRigidbody2D( Rigidbody2D* rigidbody2D )				{ m_rigidbody2D = rigidbody2D; }
	void			SetCollisionLayer( uint layer );
	const float		GetPhysicsRadius() const								{ return m_entityDef.m_physicsRadius; }
	const float		GetWalkSpeed() const									{ return m_entityDef.m_walkSpeed; }
	const float		GetMass() const											{ return m_entityDef.m_mass; }
	const float		GetOrientationDegrees() const							{ return m_orientationDegrees; }
	void			SetOrientationDegrees( float orientationDegrees )		{ m_orientationDegrees = orientationDegrees; }
	std::string		GetName() const											{ return m_entityDef.m_name; }
	std::string		GetId() const											{ return m_id; }
	void			SetId( const std::string& id ) 							{ m_id = id; }
	eEntityType		GetType() const											{ return m_entityDef.m_type; }
	const eFaction	GetFaction() const										{ return m_faction; }
	void			SetFaction( const eFaction& faction )					{ m_faction = faction; }
	void			SetMap( Map* map )										{ m_map = map; }
		
	void			AddItemToInventory( Entity* item );
	bool			IsInInventory( const std::string& itemId );
	bool			IsInInventory( Entity* item );

	void			FireSpawnEvent();
	void			FireScriptEvent( const std::string& eventName, EventArgs* args );

	bool			IsDead() const											{ return m_isDead; }
	bool			IsGarbage() const										{ return m_isGarbage; }
	bool			IsPlayer() const										{ return m_isPlayer; }
				 
	void			TakeDamage( int damage );
	//void			ApplyFriction();

	void			MoveWithPhysics( float speed, const Vec2& direction );

protected:
	void			RegisterUserEvents();
	void			FireCorrespondingEvent( EventArgs* args );

protected:
	ZephyrScript*			m_scriptObj = nullptr;

	// Game state
	const EntityDefinition& m_entityDef;
	std::string				m_id;
	eFaction				m_faction = eFaction::NEUTRAL;
	int						m_curHealth = 1;								// how much health is currently remaining on entity
	bool					m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool					m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool					m_isPlayer = false;
	Map*					m_map = nullptr;
	std::vector<Entity*>	m_inventory;									// entity owns all items in inventory

	// Physics
	Rigidbody2D*			m_rigidbody2D = nullptr;
	float					m_orientationDegrees = 0.f;						// the Entity’s forward - facing direction, as an angle in degrees
	
	// Visual
	float					m_cumulativeTime = 0.f;
	std::vector<Vertex_PCU> m_vertices;
	SpriteAnimDefinition*	m_curAnimDef = nullptr;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;
