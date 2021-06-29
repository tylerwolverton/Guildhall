#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"
#include "Engine/ZephyrCore/ZephyrEntity.hpp"
#include "Game/EntityDefinition.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Texture;


//-----------------------------------------------------------------------------------------------
class Entity : public ZephyrEntity
{
	friend class Map;
	friend class TileMap;
	
public:
	Entity( const EntityDefinition& entityDef, Map* map );
	virtual ~Entity() {}

	virtual void		Update( float deltaSeconds ) override;
	virtual void		Render() const;
	virtual void		Die() override;
	virtual void		DebugRender() const;

	const Vec2			GetForwardVector() const;
	const Vec2			GetPosition() const										{ return m_position; }
	void				SetPosition( const Vec2& position )						{ m_position = position; }
	const float			GetPhysicsRadius() const								{ return m_entityDef.m_physicsRadius; }
	const float			GetHeight() const										{ return m_entityDef.m_height; }
	const float			GetEyeHeight() const									{ return m_entityDef.m_eyeHeight; }
	const float			GetWalkSpeed() const									{ return m_entityDef.m_walkSpeed; }
	const float			GetMass() const											{ return m_entityDef.m_mass; }
	const float			GetOrientationDegrees() const							{ return m_orientationDegrees; }
	void				SetOrientationDegrees( float orientationDegrees )		{ m_orientationDegrees = orientationDegrees; }
	std::string			GetType() const											{ return m_entityDef.m_type; }
	eEntityClass		GetClass() const										{ return m_entityDef.m_class; }
	Map*				GetMap() const											{ return m_map; }
	void				SetMap( Map* map )										{ m_map = map; }

	void				AddVelocity( const Vec2& deltaVelocity )				{ m_velocity += deltaVelocity; }
	void				Translate( const Vec2& translation )					{ m_position += translation; }
	void				RotateDegrees( float pitchDegrees, float yawDegrees, float rollDegrees );

	virtual bool		IsDead() const											{ return m_isDead; }
	bool				IsGarbage() const										{ return m_isGarbage; }
	bool				IsPossessed() const										{ return m_isPossessed; }

	void				Possess()												{ m_isPossessed = true; }
	void				Unpossess()												{ m_isPossessed = false; }
						
	void				TakeDamage( int damage );
	void				ApplyFriction();

	void				RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );
	void				UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );

	// Script overrides
	virtual ZephyrValue	GetGlobalVariable( const std::string& varName ) override;
	virtual void		SetGlobalVariable( const std::string& varName, const ZephyrValue& value ) override;
	virtual void		AddGameEventParams( EventArgs* args ) const override;

protected:
	char				GetKeyCodeFromString( const std::string& keyCodeStr );

protected:
	// Game state
	const EntityDefinition& m_entityDef;
	int						m_curHealth = 1;								// how much health is currently remaining on entity
	Map*					m_map = nullptr;

	bool					m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool					m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool					m_isPossessed = false;							

	// Physics
	Vec2					m_position = Vec2( 0.f, 0.f );					// the Entity’s 2D(x, y) Cartesian origin / center location, in world space
	Vec2					m_velocity = Vec2( 0.f, 0.f );					// the Entity’s linear 2D( x, y ) velocity, in world units per second
	Vec2					m_linearAcceleration = Vec2( 0.f, 0.f );		// the Entity’s signed linear acceleration per second per second
	float					m_orientationDegrees = 0.f;						// the Entity’s forward - facing direction, as an angle in degrees
	float					m_angularVelocity = 0.f;						// the Entity’s signed angular velocity( spin rate ), in degrees per second
	bool					m_canBePushedByWalls = false;
	bool					m_canBePushedByEntities = false;
	bool					m_canPushEntities = false;

	// Visual
	float					m_cumulativeTime = 0.f;
	std::vector<Vertex_PCU> m_vertices;
	Texture*				m_texture = nullptr;

	// Input
	std::map<char, std::vector<std::string>> m_registeredKeyEvents;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;
