#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Time/Timer.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Collision2D;
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
struct DamageMultiplier
{
public:
	float defaultMultiplier = 1.f;
	float curMultiplier = 1.f;

public:
	DamageMultiplier() = default;

	DamageMultiplier( float defaultMultiplier )
		: defaultMultiplier( defaultMultiplier )
		, curMultiplier( defaultMultiplier )
	{ }

	void Reset()					{ curMultiplier = defaultMultiplier; }
};


//-----------------------------------------------------------------------------------------------
class Entity
{
	friend class Map;
	friend class TileMap;
	
public:
	Entity( const EntityDefinition& entityDef, Map* map );
	virtual ~Entity();

	virtual void			Update( float deltaSeconds );
	virtual void			Render() const;
	virtual void			Die();
	virtual void			DebugRender() const;

	virtual void			Load();
	virtual void			Unload();

	void					ChangeZephyrScriptState( const std::string& targetState );
	void					UnloadZephyrScript();
	void					ReloadZephyrScript();
	void					InitializeScriptValues( const ZephyrValueMap& initialValues );

	void					ChangeSpriteAnimation( const std::string& spriteAnimDefSetName );

	void					SetOwner( void* ownerObj )								{ m_owner = ownerObj; }
	void*					GetOwner() const										{ return m_owner; }

	const Vec2				GetForwardVector() const;
	const Vec2				GetPosition() const;
	void					SetPosition( const Vec2& position );
	void					SetInventoryItemPositions( const Vec2& position );
	void					SetRigidbody2D( Rigidbody2D* rigidbody2D )				{ m_rigidbody2D = rigidbody2D; }
	void					SetCollisionLayer( uint layer );
	const float				GetPhysicsRadius() const								{ return m_entityDef.m_physicsRadius; }
	const float				GetSpeed() const										{ return m_entityDef.m_speed; }
	const float				GetMass() const											{ return m_entityDef.m_mass; }
	const float				GetDrag() const											{ return m_entityDef.m_drag; }
	const float				GetOrientationDegrees() const							{ return m_orientationDegrees; }
	void					SetOrientationDegrees( float orientationDegrees )		{ m_orientationDegrees = orientationDegrees; }

	EntityId				GetId()													{ return m_id; }
	std::string				GetType() const											{ return m_entityDef.m_type; }
	std::string				GetName() const											{ return m_name; }
	void					SetName( const std::string& name ) 						{ m_name = name; }
	eEntityClass			GetClass() const										{ return m_entityDef.m_class; }
	const eFaction			GetFaction() const										{ return m_faction; }
	void					SetFaction( const eFaction& faction )					{ m_faction = faction; }
	Map*					GetMap()												{ return m_map; }
	void					SetMap( Map* map )										{ m_map = map; }
		
	std::vector<Entity*>	GetInventory() const									{ return m_inventory; }
	void					AddItemToInventory( Entity* item );
	void					RemoveItemFromInventory( const std::string& itemName );
	void					RemoveItemFromInventory( const EntityId& itemId );
	void					RemoveItemFromInventory( Entity* item );
	bool					IsInInventory( const std::string& itemName );
	bool					IsInInventory( const EntityId& itemId );
	bool					IsInInventory( Entity* item );

	void					FireSpawnEvent();
	void					FireScriptEvent( const std::string& eventName, EventArgs* args );

	void					MakeInvincibleToAllDamage();
	void					ResetDamageMultipliers();
	void					AddNewDamageMultiplier( const std::string& damageType, float newMultiplier );
	void					ChangeDamageMultiplier( const std::string& damageType, float newMultiplier );
	void					PermanentlyChangeDamageMultiplier( const std::string& damageType, float newDefaultMultiplier );

	bool					IsDead() const											{ return m_isDead; }
	bool					IsGarbage() const										{ return m_isGarbage; }
	bool					IsPlayer() const										{ return m_isPlayer; }
				 
	void					TakeDamage( float damage, const std::string& type = "normal" );
	//void					ApplyFriction();

	void					MoveWithPhysics( float speed, const Vec2& direction );

	void					RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );
	void					UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );

protected:
	void					EnterCollisionEvent( Collision2D collision );
	void					StayCollisionEvent( Collision2D collision );
	void					ExitCollisionEvent( Collision2D collision );
	void					EnterTriggerEvent( Collision2D collision );
	void					StayTriggerEvent( Collision2D collision );
	void					ExitTriggerEvent( Collision2D collision );
	void					SendPhysicsEventToScript( Collision2D collision, const std::string& eventName );

	char					GetKeyCodeFromString( const std::string& keyCodeStr );

protected:
	ZephyrScript*							m_scriptObj = nullptr;

	void*									m_owner = nullptr;

	// Game state
	const EntityDefinition&					m_entityDef;
	std::string								m_name;
	EntityId								m_id;
	eFaction								m_faction = eFaction::NEUTRAL;
	float									m_curHealth = 1.f;								// how much health is currently remaining on entity
	bool									m_isDead = false;								// whether the Entity is �dead� in the game; affects entity and game logic
	bool									m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool									m_isPlayer = false;
	Map*									m_map = nullptr;
	std::vector<Entity*>					m_inventory;									// entity owns all items in inventory
	std::map<std::string, DamageMultiplier>	m_damageTypeMultipliers;
	float									m_baseDamageMultiplier = 1.f;

	Entity*									m_dialoguePartner = nullptr;

	// Physics
	float									m_lastDeltaSeconds = .0016f;
	Rigidbody2D*							m_rigidbody2D = nullptr;
	float									m_orientationDegrees = 0.f;						// the Entity�s forward - facing direction, as an angle in degrees
	Vec2									m_forwardVector = Vec2( 1.f, 0.f );
	
	// Visual
	float									m_cumulativeTime = 0.f;
	std::vector<Vertex_PCU>					m_vertices;
	SpriteAnimationSetDefinition*			m_curSpriteAnimSetDef = nullptr;

	// Input
	std::map<char, std::vector<std::string>> m_registeredKeyEvents;

	// Statics
	static EntityId							s_nextEntityId;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;