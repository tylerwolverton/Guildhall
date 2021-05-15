#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Item;
class Collision2D;
class Rigidbody2D;
class SpriteAnimDefinition;
class SpriteDefinition;
class ZephyrScript;


//-----------------------------------------------------------------------------------------------
class Entity
{
	friend class Map;
	friend class TileMap;
	
public:
	Entity( const EntityDefinition& entityDef, Map* curMap );
	virtual ~Entity();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	virtual void TakeDamage( int damage );
	virtual void DebugRender() const;

	virtual void	Load();
	virtual void	Unload();

	void			ChangeZephyrScriptState( const std::string& targetState );
	void			UnloadZephyrScript();
	void			ReloadZephyrScript();

	void			CreateZephyrScript( const EntityDefinition& entityDef );
	void			InitializeZephyrEntityVariables();
	void			InitializeScriptValues( const ZephyrValueMap& initialValues );
	void			SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits );
	const ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& chunkName ) const;

	ZephyrValue		GetGlobalVariable( const std::string& varName );
	void			SetGlobalVariable( const std::string& varName, const ZephyrValue& value );
	void			SetGlobalVec2Variable( const std::string& varName, const std::string& memberName, const ZephyrValue& value );

	bool			IsScriptValid() const;
	void			SetScriptObjectValidity( bool isValid );
	std::string		GetScriptName() const;

	void			FireSpawnEvent();
	void			FireScriptEvent( const std::string& eventName, EventArgs* args = nullptr );

	const SpriteDefinition* GetUISpriteDef() const { return m_uiSpriteDef; };

	void			ChangeSpriteAnimation( const std::string& spriteAnimDefSetName );
	void			PlaySpriteAnimation( const std::string& spriteAnimDefSetName );
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const	{ return m_entityDef.GetSpriteAnimSetDef( animSetName ); }

	void			SetInitialPosition( const Vec2& pos )						{ m_initialPosition = pos; SetPosition( pos ); }

	const Vec2		GetForwardVector() const;
	const Vec2		GetPosition() const;
	void			SetPosition( const Vec2& position );
	void			SetRigidbody2D( Rigidbody2D* rigidbody2D )					{ m_rigidbody2D = rigidbody2D; }
	void			SetCollisionLayer( uint layer );
	const float		GetPhysicsRadius() const									{ return m_entityDef.m_physicsRadius; }
	const float		GetSpeed() const											{ return m_entityDef.m_speed; }
	const float		GetMass() const												{ return m_entityDef.m_mass; }
	const float		GetDrag() const												{ return m_entityDef.m_drag; }
	const float		GetOrientationDegrees() const								{ return m_orientationDegrees; }
	void			SetOrientationDegrees( float orientationDegrees )			{ m_orientationDegrees = orientationDegrees; }

	void			MarkAsGarbage()												{ m_isGarbage = true; }

	EntityId		GetId() { return m_id; }
	std::string		GetName() const												{ return m_name; }
	std::string		GetType() const												{ return m_entityDef.m_type; }
	void			SetName( const std::string& name )							{ m_name = name; }
	eEntityClass	GetClass() const											{ return m_entityDef.m_class; }
	eItemType		GetItemType() const											{ return m_entityDef.m_itemType; }
	const float		GetInteractionRadius() const								{ return m_entityDef.m_interactionRadius; }
	Vec2			GetInteractionCenter() const								{ return GetPosition() + m_entityDef.GetInteractionRadiusOffset(); }
	
	Map*			GetMap()													{ return m_map; }
	void			SetMap( Map* map )											{ m_map = map; }
				 
	// Stat accessors
	int				GetMaxHealth() const										{ return m_maxHealth; }
	IntRange		GetAttackDamageRange() const								{ return m_attackDamageRange; }
	IntRange		GetDefenseRange() const										{ return m_defenseRange; }
	float			GetAttackRange() const										{ return m_attackRange; }
	float			GetAttackSpeedModifier() const								{ return m_attackSpeedModifier; }
	float			GetCritChance() const										{ return m_critChance; }

	bool			IsDead() const												{ return m_isDead; }
	bool			IsGarbage() const											{ return m_isGarbage; }
	bool			IsPlayer() const											{ return m_isPlayer; }
					
	void			MoveWithPhysics( float speed, const Vec2& direction );
	void			ApplyImpulseAt( const Vec2& impulse );

	void			RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );
	void			UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );

	void			SetAsPlayer();
	void			Respawn();

	void			GainXP( int xpValue );

	void			AddEquipment( Item* newEquipment );
	void			RemoveEquipment( Item* equipment );

	int				GetTotalMaxHealth() const;
	IntRange		GetTotalAttackDamageRange() const;
	IntRange		GetTotalDefenseRange() const;
	float			GetTotalAttackRange() const;
	float			GetTotalAttackSpeed() const;
	float			GetTotalCritChance() const;

	void			ActivateInvincibility()										{ m_isInvincible = true; }
	void			DeactivateInvincibility()									{ m_isInvincible = false; }

protected:
	void			EnterCollisionEvent( Collision2D collision );
	void			StayCollisionEvent( Collision2D collision );
	void			ExitCollisionEvent( Collision2D collision );
	void			EnterTriggerEvent( Collision2D collision );
	void			StayTriggerEvent( Collision2D collision );
	void			ExitTriggerEvent( Collision2D collision );
	void			SendPhysicsEventToScript( Collision2D collision, const std::string& eventName );

	void			UpdateUI();
	
	void			UpdateFromKeyboard( float deltaSeconds );
	void			UpdateFromMouse();

	char			GetKeyCodeFromString( const std::string& keyCodeStr );

protected:
	ZephyrScript*							m_scriptObj = nullptr;

	// Game state
	const EntityDefinition&					m_entityDef;
	std::string								m_name;
	EntityId								m_id = -1;
	Map*									m_map = nullptr;
	bool									m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool									m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool									m_isPlayer = false;
	int										m_controllerID = -1;
	bool									m_isInvincible = false;

	// Physics
	Vec2									m_initialPosition = Vec2( 0.f, 0.f );			// the Entity’s 2D(x, y) Cartesian origin / center location, in world space
	float									m_lastDeltaSeconds = .0016f;
	Rigidbody2D*							m_rigidbody2D = nullptr;
	float									m_orientationDegrees = 0.f;						// the Entity’s forward - facing direction, as an angle in degrees
	Vec2									m_forwardVector = Vec2( 1.f, 0.f );

	// Stats
	int										m_curHealth = 1;								
	int										m_maxHealth = 1;		
	IntRange								m_attackDamageRange;
	IntRange								m_defenseRange;
	float									m_attackRange = 0.f;
	float									m_attackSpeedModifier = 0.f;
	float									m_critChance = 0;
	int										m_curXP = 0;
	int										m_xpToNextLevel = 1;
	int										m_curLevel = 1;
	
	// Visual
	float									m_cumulativeTime = 0.f;
	Vec2									m_facingDirection = Vec2::ZERO_TO_ONE;
	std::vector<Vertex_PCU>					m_vertices;
	SpriteAnimationSetDefinition*			m_curSpriteAnimSetDef = nullptr;
	const SpriteDefinition*					m_uiSpriteDef = nullptr;

	// Equipment
	std::vector<Item*>						m_equipment;
	int										m_equipMaxHealth = 0;
	IntRange								m_equipAttackDamageRange;
	IntRange								m_equipDefenseRange;
	float									m_equipAttackRange = 0.f;
	float									m_equipAttackSpeedModifier = 0.f;
	float									m_equipCritChance = 0;

	// Input
	std::map<char, std::vector<std::string>> m_registeredKeyEvents;

	// Statics
	static EntityId							s_nextEntityId;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;
