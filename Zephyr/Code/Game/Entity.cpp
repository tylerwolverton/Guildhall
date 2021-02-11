#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Scripting/ZephyrScript.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
EntityId Entity::s_nextEntityId = 1000;


//-----------------------------------------------------------------------------------------------
Entity::Entity( const EntityDefinition& entityDef, Map* map )
	: m_entityDef( entityDef )
	, m_map( map )
{
	m_id = s_nextEntityId++;
	m_curHealth = m_entityDef.GetMaxHealth();

	m_rigidbody2D = g_physicsSystem2D->CreateRigidbody();
	m_rigidbody2D->SetMass( m_entityDef.GetMass() );
	
	m_rigidbody2D->SetDrag( m_entityDef.GetDrag() );
	m_rigidbody2D->SetLayer( m_entityDef.GetCollisionLayer() );
	
	switch( m_entityDef.GetCollisionLayer() )
	{
		case eCollisionLayer::STATIC_ENVIRONMENT: m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_STATIC ); break;
		case eCollisionLayer::NPC: m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_KINEMATIC ); break;
		default: m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_DYNAMIC ); break;
	}

	m_rigidbody2D->m_userProperties.SetValue( "entity", (void*)this );

	ZephyrScriptDefinition* scriptDef = entityDef.GetZephyrScriptDefinition();
	if ( scriptDef != nullptr )
	{
		m_scriptObj = new ZephyrScript( *scriptDef, this );
		m_scriptObj->InitializeGlobalVariables( entityDef.GetZephyrScriptInitialValues() );
	}

	m_curSpriteAnimSetDef = m_entityDef.GetDefaultSpriteAnimSetDef();
}


//-----------------------------------------------------------------------------------------------
Entity::~Entity()
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->Destroy();
		m_rigidbody2D = nullptr;
	}

	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_scriptObj );

	PTR_VECTOR_SAFE_DELETE( m_inventory );
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
	m_lastDeltaSeconds = deltaSeconds;

	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->Update();
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef == nullptr
		 || m_rigidbody2D == nullptr )
	{
		return;
	}

	animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_rigidbody2D->GetVelocity() );
	
	const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_entityDef.m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, GetPosition() );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	if ( IsDead() )
	{
		return;
	}

	m_isDead = true;

	if ( m_scriptObj != nullptr )
	{
		EventArgs args;
		m_scriptObj->FireEvent( "Died", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, GetPosition(), m_entityDef.m_physicsRadius, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	DrawAABB2Outline( g_renderer, GetPosition(), m_entityDef.m_localDrawBounds, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
	DrawRing2D( g_renderer, GetPosition() + m_forwardVector * ( GetPhysicsRadius() + .1f ), .1f, Rgba8::GREEN, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetPosition() const
{
	if ( m_rigidbody2D != nullptr )
	{
		return m_rigidbody2D->GetPosition();
	}

	return Vec2::ZERO;
}


//-----------------------------------------------------------------------------------------------
void Entity::SetPosition( const Vec2& position )
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->SetPosition( position );

		EventArgs args;
		args.SetValue( "newPos", m_rigidbody2D->GetPosition() );

		FireScriptEvent( "PositionUpdated", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::SetCollisionLayer( uint layer )
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->SetLayer( layer );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::AddItemToInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] == nullptr )
		{
			m_inventory[itemIdx] = item;
			return;
		}
	}

	m_inventory.push_back( item );

	EventArgs args;
	args.SetValue( "itemName", item->GetName() );

	g_eventSystem->FireEvent( "ItemAcquired", &args );
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveItemFromInventory( const std::string& itemName )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx]->GetName() == itemName )
		{
			m_inventory[itemIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveItemFromInventory( const EntityId& itemId )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx]->GetId() == itemId )
		{
			m_inventory[itemIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( const EntityId& itemId )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx]->GetId() == itemId )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveItemFromInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx] == item )
		{
			m_inventory[itemIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( const std::string& itemName )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			&& m_inventory[itemIdx]->GetName() == itemName )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			&& m_inventory[itemIdx] == item )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Entity::FireSpawnEvent()
{
	if ( m_scriptObj != nullptr )
	{
		EventArgs args;
		args.SetValue( "maxHealth", m_entityDef.GetMaxHealth() );
		args.SetValue( "entityId", GetId() );
		args.SetValue( "entityName", GetName() );

		m_scriptObj->FireEvent( "Spawned", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::FireScriptEvent( const std::string& eventName, EventArgs* args )
{
	if ( m_scriptObj == nullptr )
	{
		return;
	}

	m_scriptObj->FireEvent( eventName, args );
}


//-----------------------------------------------------------------------------------------------
void Entity::MakeInvincibleToAllDamage()
{
	m_baseDamageMultiplier = 0.f;
	/*for ( auto& damageMultiplier : m_damageTypeMultipliers )
	{
		damageMultiplier.second.curMultiplier = 0.f;
	}*/
}


//-----------------------------------------------------------------------------------------------
void Entity::ResetDamageMultipliers()
{
	m_baseDamageMultiplier = 1.f;

	for ( auto& damageMultiplier : m_damageTypeMultipliers )
	{
		damageMultiplier.second.curMultiplier = damageMultiplier.second.defaultMultiplier;
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::AddNewDamageMultiplier( const std::string& damageType, float newMultiplier )
{
	m_damageTypeMultipliers[damageType] = DamageMultiplier( newMultiplier );
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeDamageMultiplier( const std::string& damageType, float newMultiplier )
{
	auto damageIter = m_damageTypeMultipliers.find( damageType );
	if ( damageIter == m_damageTypeMultipliers.end() )
	{
		g_devConsole->PrintError( Stringf( "Tried to change multiplier of unknown damage type '%s'", damageType.c_str() ) );
	}

	damageIter->second.curMultiplier = newMultiplier;
}


//-----------------------------------------------------------------------------------------------
void Entity::PermanentlyChangeDamageMultiplier( const std::string& damageType, float newDefaultMultiplier )
{
	auto damageIter = m_damageTypeMultipliers.find( damageType );
	if ( damageIter == m_damageTypeMultipliers.end() )
	{
		g_devConsole->PrintError( Stringf( "Tried to permanently change multiplier of unknown damage type '%s'", damageType.c_str() ) );
	}

	damageIter->second.defaultMultiplier = newDefaultMultiplier;
	damageIter->second.curMultiplier = newDefaultMultiplier;
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( float damage, const std::string& type )
{
	if ( IsDead() )
	{
		return;
	}

	float damageMultiplier = 1.f;

	auto damageIter = m_damageTypeMultipliers.find( type );
	if ( damageIter != m_damageTypeMultipliers.end() )
	{
		damageMultiplier = damageIter->second.curMultiplier;
	}

	m_curHealth -= damage * damageMultiplier * m_baseDamageMultiplier;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	if ( m_scriptObj != nullptr )
	{
		EventArgs args;
		args.SetValue( "newHealth", m_curHealth );

		m_scriptObj->FireEvent( "HealthUpdated", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::MoveWithPhysics( float speed, const Vec2& direction )
{
	if ( m_rigidbody2D != nullptr )
	{
		m_rigidbody2D->ApplyImpulseAt( speed * direction * m_lastDeltaSeconds, GetPosition() );
		m_forwardVector = direction;

		EventArgs args;
		args.SetValue( "newPos", m_rigidbody2D->GetPosition() );

		FireScriptEvent( "PositionUpdated", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );
	
	if ( keyCode == '\0' )
	{
		return;
	}

	m_registeredKeyEvents[keyCode].push_back( eventName );
}


//-----------------------------------------------------------------------------------------------
void Entity::UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );

	if ( keyCode == '\0' )
	{
		return;
	}

	auto eventIter = m_registeredKeyEvents.find(keyCode);
	if ( eventIter == m_registeredKeyEvents.end() )
	{
		return;
	}

	Strings& eventNames = eventIter->second;
	int eventIdx = 0;
	for ( ; eventIdx < (int)eventNames.size(); ++eventIdx )
	{
		if ( eventName == eventNames[eventIdx] )
		{
			break;
		}
	}

	// Remove bound event
	if ( eventIdx < (int)eventNames.size() )
	{
		eventNames.erase( eventNames.begin() + eventIdx );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Load()
{
	m_rigidbody2D->Enable();

	if ( m_entityDef.IsTrigger() )
	{
		DiscCollider2D* discTrigger = g_physicsSystem2D->CreateDiscTrigger( Vec2::ZERO, GetPhysicsRadius() );
		
		discTrigger->m_onTriggerEnterDelegate.SubscribeMethod( this, &Entity::EnterTriggerEvent );
		discTrigger->m_onTriggerStayDelegate.SubscribeMethod( this, &Entity::StayTriggerEvent );
		discTrigger->m_onTriggerLeaveDelegate.SubscribeMethod( this, &Entity::ExitTriggerEvent );

		m_rigidbody2D->TakeCollider( discTrigger );
	}
	else
	{
		DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscCollider( Vec2::ZERO, GetPhysicsRadius() );

		discCollider->m_onOverlapEnterDelegate.SubscribeMethod( this, &Entity::EnterCollisionEvent );
		discCollider->m_onOverlapStayDelegate.SubscribeMethod( this, &Entity::StayCollisionEvent );
		discCollider->m_onOverlapLeaveDelegate.SubscribeMethod( this, &Entity::ExitCollisionEvent );

		m_rigidbody2D->TakeCollider( discCollider );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Unload()
{
	m_rigidbody2D->Disable();

	g_physicsSystem2D->DestroyCollider( m_rigidbody2D->GetCollider() );
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeZephyrScriptState( const std::string& targetState )
{
	if ( m_scriptObj == nullptr )
	{
		g_devConsole->PrintWarning( Stringf( "Tried to change state of entity: %s to %s, but it doesn't have a script", m_name.c_str(), targetState.c_str() ) );
		return;
	}

	m_scriptObj->ChangeState( targetState );
}


//-----------------------------------------------------------------------------------------------
void Entity::UnloadZephyrScript()
{
	if ( m_scriptObj == nullptr )
	{
		return;
	}

	m_scriptObj->UnloadScript();
}


//-----------------------------------------------------------------------------------------------
void Entity::ReloadZephyrScript()
{
	if ( m_scriptObj != nullptr )
	{
		PTR_SAFE_DELETE( m_scriptObj );

		ZephyrScriptDefinition* scriptDef = m_entityDef.GetZephyrScriptDefinition();
		if ( scriptDef != nullptr )
		{
			m_scriptObj = new ZephyrScript( *scriptDef, this );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::InitializeScriptValues( const ZephyrValueMap& initialValues )
{
	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->InitializeGlobalVariables( initialValues );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", m_name.c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
}


//-----------------------------------------------------------------------------------------------
void Entity::EnterCollisionEvent( Collision2D collision )
{
	SendPhysicsEventToScript( collision, "CollisionEntered" );
}


//-----------------------------------------------------------------------------------------------
void Entity::StayCollisionEvent( Collision2D collision )
{
	SendPhysicsEventToScript( collision, "CollisionStayed" );
}


//-----------------------------------------------------------------------------------------------
void Entity::ExitCollisionEvent( Collision2D collision )
{
	SendPhysicsEventToScript( collision, "CollisionExited" );
}


//-----------------------------------------------------------------------------------------------
void Entity::EnterTriggerEvent( Collision2D collision )
{
	SendPhysicsEventToScript( collision, "TriggerEntered" );
}


//-----------------------------------------------------------------------------------------------
void Entity::StayTriggerEvent( Collision2D collision )
{
	SendPhysicsEventToScript( collision, "TriggerStayed" );
}


//-----------------------------------------------------------------------------------------------
void Entity::ExitTriggerEvent( Collision2D collision )
{
	SendPhysicsEventToScript( collision, "TriggerExited" );
}


//-----------------------------------------------------------------------------------------------
void Entity::SendPhysicsEventToScript( Collision2D collision, const std::string& eventName )
{
	if ( !IsDead() )
	{
		Entity* theirEntity = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entity", ( void* )nullptr );

		if ( m_scriptObj != nullptr )
		{
			EventArgs args;
			EntityId otherId = -1;
			std::string otherName;
			std::string otherType;
			if ( theirEntity != nullptr
				 && !theirEntity->IsDead() )
			{
				otherId = theirEntity->GetId();
				otherName = theirEntity->GetName();
				otherType = theirEntity->GetType();
			}

			args.SetValue( "otherEntityId", otherId );
			args.SetValue( "otherEntityName", otherName );
			args.SetValue( "otherEntityType", otherType );
			m_scriptObj->FireEvent( eventName, &args );
		}
	}
}


//-----------------------------------------------------------------------------------------------
char Entity::GetKeyCodeFromString( const std::string& keyCodeStr )
{
	if ( IsEqualIgnoreCase( keyCodeStr, "space" ) )			{ return KEY_SPACEBAR; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "enter" ) )	{ return KEY_ENTER; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "shift" ) )	{ return KEY_SHIFT; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "left" ) )		{ return KEY_LEFTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "right" ) )	{ return KEY_RIGHTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "up" ) )		{ return KEY_UPARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "down" ) )		{ return KEY_DOWNARROW; }
	else
	{
		// Register letters and numbers
		char key = keyCodeStr[0];
		if ( key >= 'a' && key <= 'z' )
		{
			key -= 32;
		}

		if ( ( key >= '0' && key <= '9' )
			 || ( key >= 'A' && key <= 'Z' ) )
		{
			return key;
		}
	}

	return '\0';
}

