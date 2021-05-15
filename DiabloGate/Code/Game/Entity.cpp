#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIElement.hpp"
#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UIUniformGrid.hpp"

#include "Game/EntityDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Item.hpp"
#include "Game/Map.hpp"
#include "Game/LevelThresholdDefinition.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Scripting/ZephyrScript.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
EntityId Entity::s_nextEntityId = 1000;


//-----------------------------------------------------------------------------------------------
Entity::Entity( const EntityDefinition& entityDef, Map* curMap )
	: m_entityDef( entityDef )
	, m_map( curMap )
{
	m_id = s_nextEntityId++;

	// Stats
	m_maxHealth = m_entityDef.GetInitialMaxHealth();
	m_curHealth = m_entityDef.GetInitialMaxHealth();
	m_attackDamageRange = m_entityDef.GetInitialAttackDamageRange();
	m_defenseRange = m_entityDef.GetInitialDefenseRange();
	m_attackRange = m_entityDef.GetInitialAttackRange();
	m_attackSpeedModifier = m_entityDef.GetInitialAttackSpeedMultiplier();
	m_critChance = m_entityDef.GetInitialCritChance();

	// Physics
	m_rigidbody2D = g_physicsSystem2D->CreateRigidbody();
	m_rigidbody2D->SetMass( m_entityDef.GetMass() );

	m_rigidbody2D->SetDrag( m_entityDef.GetDrag() );
	m_rigidbody2D->SetLayer( m_entityDef.GetCollisionLayer() );

	switch ( m_entityDef.GetCollisionLayer() )
	{
		case eCollisionLayer::STATIC_ENVIRONMENT: m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_STATIC ); break;
		case eCollisionLayer::NPC: m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_KINEMATIC ); break;
		default: m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_DYNAMIC ); break;
	}

	if ( m_entityDef.GetSimMode() != eSimulationMode::SIMULATION_MODE_NONE )
	{
		m_rigidbody2D->SetSimulationMode( m_entityDef.GetSimMode() );
	}

	m_rigidbody2D->m_userProperties.SetValue( "entity", (void*)this );

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

	Unload();

	// Animation
	m_curSpriteAnimSetDef = m_entityDef.GetDefaultSpriteAnimSetDef();
	m_cumulativeTime += g_game->m_rng->RollRandomFloatInRange( 0.f, .5f );
	
	SpriteAnimationSetDefinition* animSetDef = entityDef.GetSpriteAnimSetDef( "UI" );
	if ( animSetDef != nullptr )
	{
		m_uiSpriteDef = &( animSetDef->GetSpriteAnimationDefForDirection( Vec2::ONE )->GetSpriteDefAtTime( 0.f ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::CreateZephyrScript( const EntityDefinition& entityDef )
{
	ZephyrScriptDefinition* scriptDef = entityDef.GetZephyrScriptDefinition();
	if ( scriptDef != nullptr )
	{
		m_scriptObj = new ZephyrScript( *scriptDef, this );
		m_scriptObj->InterpretGlobalBytecodeChunk();
		m_scriptObj->InitializeGlobalVariables( entityDef.GetZephyrScriptInitialValues() );
		m_scriptObj->SetEntityVariableInitializers( entityDef.GetZephyrEntityVarInits() );
	}
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
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{	
	m_cumulativeTime += deltaSeconds;
	m_lastDeltaSeconds = deltaSeconds;

	if ( !m_isDead )
	{
		if ( m_isPlayer )
		{
			UpdateFromKeyboard( deltaSeconds );
		}
	}

	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->Update();
	}
	
	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef != nullptr )
	{
		animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_facingDirection );
		int frameIndex = animDef->GetFrameIndexAtTime( m_cumulativeTime );

		m_curSpriteAnimSetDef->FireFrameEvent( frameIndex, this );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_devConsole->IsOpen() 
		 || m_isDead 
		 || g_game->GetGameState() != eGameState::PLAYING )
	{
		return;
	}

	UpdateFromMouse();

	// Update script button events
	for ( auto& registeredKey : m_registeredKeyEvents )
	{
		if ( g_inputSystem->IsKeyPressed( registeredKey.first ) )
		{
			for ( auto& eventName : registeredKey.second )
			{
				EventArgs args;
				FireScriptEvent( eventName, &args );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdateFromMouse()
{
	if ( g_game->GetUISystem()->IsMouseCursorInUI() )
	{
		return;
	}

	if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON )
		 || g_inputSystem->WasKeyJustPressed( MOUSE_RBUTTON ) )
	{
		Entity* entityUnderMouse = m_map->GetEntityUnderMouse( std::vector<Entity*>{ this } );
		EntityId entityUnderMouseId = (EntityId)-1;
		if ( entityUnderMouse != nullptr
			 && !entityUnderMouse->IsDead() )
		{
			entityUnderMouseId = entityUnderMouse->GetId();
		}

		EventArgs args;
		args.SetValue( "worldPosition", g_game->GetMouseWorldPosition() );
		args.SetValue( "clickedEntity", (EntityId)entityUnderMouseId );
		FireScriptEvent( "OnClickInWorld", &args );
	}
}


//-----------------------------------------------------------------------------------------------
char Entity::GetKeyCodeFromString( const std::string& keyCodeStr )
{

	if ( IsEqualIgnoreCase( keyCodeStr, "space" ) ) { return KEY_SPACEBAR; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "enter" ) ) { return KEY_ENTER; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "shift" ) ) { return KEY_SHIFT; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "left" ) ) { return KEY_LEFTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "right" ) ) { return KEY_RIGHTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "up" ) ) { return KEY_UPARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "down" ) ) { return KEY_DOWNARROW; }
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


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef == nullptr
		 || m_rigidbody2D == nullptr )
	{
		return;
	}

	animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_forwardVector );
	
	const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_entityDef.m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, GetPosition() );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );

	// Draw health bar if damaged
	if ( m_isPlayer 
		 || m_isDead
		 || m_curHealth == m_maxHealth )
	{
		return;
	}

	std::vector<Vertex_PCU> hpVertices;
	Vec2 hpBarMins( m_entityDef.m_localDrawBounds.mins * .5f );
	Vec2 hpBarMaxs( m_entityDef.m_localDrawBounds.maxs.x, m_entityDef.m_localDrawBounds.mins.y + .065f );
	hpBarMaxs *= .5f;

	AABB2 hpBarBackground( hpBarMins, hpBarMaxs );

	AppendVertsForAABB2D( hpVertices, hpBarBackground, Rgba8::BLACK );

	AABB2 hpRemaining( hpBarBackground );
	float hpFraction = (float)m_curHealth / (float)m_maxHealth;
	hpRemaining.ChopOffRight( 1.f - hpFraction, 0.f );

	Rgba8 hpColor = Rgba8::GREEN;
	if ( hpFraction < .25f )
	{
		hpColor = Rgba8::RED;
	}
	else if ( hpFraction < .5f )
	{
		hpColor = Rgba8::YELLOW;
	}

	AppendVertsForAABB2D( hpVertices, hpRemaining, hpColor );

	Vec2 hpPosition( GetPosition() );
	hpPosition.y += m_entityDef.m_localDrawBounds.GetHeight() * .6f;

	Vertex_PCU::TransformVertexArray( hpVertices, 1.f, 0.f, hpPosition );

	g_renderer->BindDiffuseTexture( nullptr );
	g_renderer->DrawVertexArray( hpVertices );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	if ( IsDead() )
	{
		return;
	}

	if ( !m_isPlayer )
	{
		g_game->GetPlayer()->GainXP( m_entityDef.GetXPReward() );
	}

	for ( int lootDropIdx = 0; lootDropIdx <  (int)m_entityDef.GetLootDrops().size(); ++lootDropIdx )
	{
		LootDrop lootDrop = m_entityDef.GetLootDrops()[lootDropIdx];
		
		if ( g_game->m_rng->RollPercentChance( lootDrop.dropRate ) )
		{
			Entity* newEntity = m_map->SpawnNewEntityOfType( lootDrop.entityDefName );
			if ( newEntity != nullptr )
			{
				newEntity->InitializeZephyrEntityVariables();
				newEntity->Load();
				newEntity->SetPosition( GetPosition() );
				newEntity->ApplyImpulseAt( g_game->m_rng->RollRandomDirection2D() * g_game->m_rng->RollRandomFloatInRange( 55.f, 70.f ) );
			}
		}
	}

	m_isDead = true;
	
	Unload();

	// Change this later to allow for death anims
	//m_isGarbage = true;

	if ( IsScriptValid() )
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
	DrawRing2D( g_renderer, GetInteractionCenter(), m_entityDef.m_interactionRadius, Rgba8::YELLOW, DEBUG_LINE_THICKNESS );
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
void Entity::TakeDamage( int damage )
{
	if ( m_isInvincible )
	{
		return;
	}

	m_curHealth -= damage;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	if ( m_curHealth > m_maxHealth )
	{
		m_curHealth = m_maxHealth;
	}

	if ( m_isPlayer )
	{
		UpdateUI();
	}

	if ( m_scriptObj != nullptr )
	{
		EventArgs args;
		args.SetValue( "newHealth", m_curHealth );

		m_scriptObj->FireEvent( "HealthUpdated", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::FireSpawnEvent()
{
	if ( IsScriptValid() )
	{
		EventArgs args;
		args.SetValue( "maxHealth", m_entityDef.GetInitialMaxHealth() );
		args.SetValue( "entityId", GetId() );
		args.SetValue( "entityName", GetName() );

		m_scriptObj->FireEvent( "Spawned", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::FireScriptEvent( const std::string& eventName, EventArgs* args )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_scriptObj->FireEvent( eventName, args );
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
void Entity::ApplyImpulseAt( const Vec2& impulse )
{
	m_rigidbody2D->ApplyImpulseAt( impulse, GetPosition() );
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

	auto eventIter = m_registeredKeyEvents.find( keyCode );
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
void Entity::SetAsPlayer()
{
	m_controllerID = 0;
	m_isPlayer = true;

	m_name = "player";
	m_rigidbody2D->SetLayer( eCollisionLayer::PLAYER );
	m_rigidbody2D->Enable();
	//DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscCollider( Vec2::ZERO, GetPhysicsRadius() );
	//m_rigidbody2D->TakeCollider( discCollider );

	m_xpToNextLevel = LevelThresholdDefinition::GetXPToLevel2();

	UpdateUI();
}


//-----------------------------------------------------------------------------------------------
void Entity::Respawn()
{
	if ( m_isPlayer )
	{
		m_isDead = false;
		m_isGarbage = false;
		Load();

		SetPosition( m_initialPosition );

		m_curHealth = GetMaxHealth();
		UpdateUI();

		g_game->SetWorldCameraPosition( Vec3( m_rigidbody2D->GetPosition(), 0.f ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::GainXP( int xpValue )
{
	m_curXP += xpValue;

	LevelData levelData = LevelThresholdDefinition::GetLevelForXP( m_curXP );

	if ( levelData.num > m_curLevel )
	{
		++m_curLevel;
		m_xpToNextLevel = levelData.xpToNextLevel;
		m_attackDamageRange.min += levelData.dmgIncrease.x;
		m_attackDamageRange.max += levelData.dmgIncrease.y;
		m_defenseRange.min += levelData.defenseIncrease.x;
		m_defenseRange.max += levelData.defenseIncrease.y;
		m_attackRange += levelData.rangeIncrease;
		m_attackSpeedModifier += levelData.atkSpeedIncrease;
		m_critChance += levelData.critChanceIncrease;

		SpriteAnimationSetDefinition* attackSpriteAnim = m_entityDef.GetSpriteAnimSetDef( "Attack" );

		if ( attackSpriteAnim != nullptr )
		{
			attackSpriteAnim->AdjustAnimationSpeed( levelData.atkSpeedIncrease );
		}
	}

	UpdateUI();
}


//-----------------------------------------------------------------------------------------------
void Entity::AddEquipment( Item* newEquipment )
{
	if ( newEquipment == nullptr )
	{
		return;
	}

	m_equipMaxHealth += newEquipment->GetMaxHealth();
	m_equipAttackDamageRange = m_equipAttackDamageRange + newEquipment->GetAttackDamageRange();
	//m_equipAttackRange += newEquipment->GetAttackRange();
	m_equipDefenseRange = m_equipDefenseRange + newEquipment->GetDefenseRange();
	m_equipAttackSpeedModifier = m_equipAttackSpeedModifier + newEquipment->GetAttackSpeedModifier();
	m_equipCritChance += newEquipment->GetCritChance();

	if ( !IsNearlyEqual( newEquipment->GetAttackSpeedModifier(), 0.f ) )
	{
		SpriteAnimationSetDefinition* attackSpriteAnim = m_entityDef.GetSpriteAnimSetDef( "Attack" );

		if ( attackSpriteAnim != nullptr )
		{
			attackSpriteAnim->AdjustAnimationSpeed( newEquipment->GetAttackSpeedModifier() );
		}
	}

	UpdateUI();

	for ( int equipIdx = 0; equipIdx < (int)m_equipment.size(); ++equipIdx )
	{
		if ( m_equipment[equipIdx] == nullptr )
		{
			m_equipment[equipIdx] = newEquipment;
			return;
		}
	}

	m_equipment.push_back( newEquipment );
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveEquipment( Item* equipment )
{
	if ( equipment == nullptr )
	{
		return;
	}

	for ( int equipIdx = 0; equipIdx < (int)m_equipment.size(); ++equipIdx )
	{
		if ( m_equipment[equipIdx] == equipment )
		{
			m_equipment[equipIdx] = nullptr;

			m_equipMaxHealth -= equipment->GetMaxHealth();
			m_equipAttackDamageRange = m_equipAttackDamageRange - equipment->GetAttackDamageRange();
			m_equipDefenseRange = m_equipDefenseRange - equipment->GetDefenseRange();
			m_equipAttackRange -= equipment->GetAttackRange();
			m_equipAttackSpeedModifier = m_equipAttackSpeedModifier - equipment->GetAttackSpeedModifier();
			m_equipCritChance -= equipment->GetCritChance();

			if ( !IsNearlyEqual( equipment->GetAttackSpeedModifier(), 0.f ) )
			{
				SpriteAnimationSetDefinition* attackSpriteAnim = m_entityDef.GetSpriteAnimSetDef( "Attack" );

				if ( attackSpriteAnim != nullptr )
				{
					attackSpriteAnim->AdjustAnimationSpeed( -equipment->GetAttackSpeedModifier() );
				}
			}

			UpdateUI();

			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Load()
{
	if ( m_isDead )
	{
		return;
	}

	m_rigidbody2D->Enable();
}


//-----------------------------------------------------------------------------------------------
void Entity::Unload()
{
	if ( m_rigidbody2D == nullptr  )
	{
		return;
	}

	m_rigidbody2D->Disable();
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeZephyrScriptState( const std::string& targetState )
{
	if ( !IsScriptValid() )
	{
		g_devConsole->PrintWarning( Stringf( "Tried to change state of entity: %s to %s, but it doesn't have a valid script", m_name.c_str(), targetState.c_str() ) );
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
	if ( IsScriptValid() )
	{
		m_scriptObj->InitializeGlobalVariables( initialValues );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits )
{
	if ( IsScriptValid() )
	{
		m_scriptObj->SetEntityVariableInitializers( entityVarInits );
	}
}


//-----------------------------------------------------------------------------------------------
const ZephyrBytecodeChunk* Entity::GetBytecodeChunkByName( const std::string& chunkName ) const
{
	if ( m_scriptObj == nullptr )
	{
		return nullptr;
	}

	return m_scriptObj->GetBytecodeChunkByName( chunkName );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue Entity::GetGlobalVariable( const std::string& varName )
{
	// First check c++ built in vars
	if ( varName == "id" )					{ return ZephyrValue( (float)GetId() ); }
	if ( varName == "name" )				{ return ZephyrValue( GetName() ); }
	if ( varName == "class" )				{ return ZephyrValue( GetEntityClassAsString( GetClass() ) );	}
	if ( varName == "health" )				{ return ZephyrValue( (float)m_curHealth ); }
	if ( varName == "maxHealth" )			{ return ZephyrValue( (float)m_entityDef.GetInitialMaxHealth() ); }
	if ( varName == "position" )			{ return ZephyrValue( GetPosition() ); }
	if ( varName == "forwardVec" )			{ return ZephyrValue( GetForwardVector() ); }
	if ( varName == "speed" )				{ return ZephyrValue( GetSpeed() ); }
	if ( varName == "interactionRadius" )	{ return ZephyrValue( GetInteractionRadius() ); }
	if ( varName == "interactionCenter" )	{ return ZephyrValue( GetInteractionCenter() ); }

	if ( varName == "attackDamage" )		{ return ZephyrValue( (float)( GetTotalAttackDamageRange().GetRandomInRange( g_game->m_rng ) ) ); }
	if ( varName == "defense" )				{ return ZephyrValue( (float)( GetTotalDefenseRange().GetRandomInRange( g_game->m_rng ) ) ); }
	if ( varName == "attackSpeedModifier" )	{ return ZephyrValue( GetAttackSpeedModifier() ); }
	if ( varName == "isDead" )				{ return ZephyrValue( m_isDead ); }

	if ( !IsScriptValid() )
	{
		return ZephyrValue( ERROR_ZEPHYR_VAL );
	}

	return m_scriptObj->GetGlobalVariable( varName );
}


//-----------------------------------------------------------------------------------------------
void Entity::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	// First check c++ built in vars
	if ( varName == "health" )
	{
		m_curHealth = (int)value.GetAsNumber();
		if ( m_curHealth < 0 )
		{
			Die();
		}
	}

	m_scriptObj->SetGlobalVariable( varName, value );
}


//-----------------------------------------------------------------------------------------------
void Entity::SetGlobalVec2Variable( const std::string& varName, const std::string& memberName, const ZephyrValue& value )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_scriptObj->SetGlobalVec2Variable( varName, memberName, value );
}


//-----------------------------------------------------------------------------------------------
void Entity::InitializeZephyrEntityVariables()
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_scriptObj->InitializeEntityVariables();
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsScriptValid() const
{
	if ( m_scriptObj == nullptr )
	{
		return false;
	}

	return m_scriptObj->IsScriptValid();
}


//-----------------------------------------------------------------------------------------------
void Entity::SetScriptObjectValidity( bool isValid )
{
	if ( m_scriptObj == nullptr )
	{
		return;
	}

	return m_scriptObj->SetScriptObjectValidity( isValid );
}


//-----------------------------------------------------------------------------------------------
std::string Entity::GetScriptName() const
{
	if ( m_scriptObj == nullptr )
	{
		return "unknown";
	}

	return m_scriptObj->GetScriptName();
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", GetName().c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
}


//-----------------------------------------------------------------------------------------------
void Entity::PlaySpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", GetName().c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
	m_cumulativeTime = 0.f;
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
	if ( !IsDead()
		 && collision.theirCollider != nullptr )
	{
		Entity* theirEntity = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entity", ( void* )nullptr );

		if ( IsScriptValid() )
		{
			// Save data if collision is with another entity
			// Still report collision regardless to account for environmental collisions
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

			args.SetValue( "otherEntity", otherId );
			args.SetValue( "otherEntityName", otherName );
			args.SetValue( "otherEntityType", otherType );
			m_scriptObj->FireEvent( eventName, &args );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdateUI()
{
	UIElement* attackDamageLabel = g_game->GetUISystem()->GetUIElementByName( "attackDamageLabel" );
	if ( attackDamageLabel != nullptr )
	{
		( (UIText*)attackDamageLabel )->SetText( Stringf( "Atk Dmg: %i - %i", GetTotalAttackDamageRange().min, GetTotalAttackDamageRange().max ) );
	}

	UIElement* defenseLabel = g_game->GetUISystem()->GetUIElementByName( "defenseLabel" );
	if ( defenseLabel != nullptr )
	{
		( (UIText*)defenseLabel )->SetText( Stringf( "Defense:  %i - %i", GetTotalDefenseRange().min, GetTotalDefenseRange().max ) );
	}

	UIElement* attackSpeedLabel = g_game->GetUISystem()->GetUIElementByName( "attackSpeedLabel" );
	if ( attackSpeedLabel != nullptr )
	{
		( (UIText*)attackSpeedLabel )->SetText( Stringf( "Atk Speed: x%.2f", GetTotalAttackSpeed() ) );
	}

	UIElement* critChanceLabel = g_game->GetUISystem()->GetUIElementByName( "critChanceLabel" );
	if ( critChanceLabel != nullptr )
	{
		( (UIText*)critChanceLabel )->SetText( "Crit Chance: " + ToString( GetTotalCritChance() ) + "%" );
	}

	UIElement* levelLabel = g_game->GetUISystem()->GetUIElementByName( "levelLabel" );
	if ( levelLabel != nullptr )
	{
		( (UIText*)levelLabel )->SetText( "Level " + ToString( m_curLevel ) );
	}

	UIElement* xpLabel = g_game->GetUISystem()->GetUIElementByName( "xpLabel" );
	if ( xpLabel != nullptr )
	{
		std::string xpValue = m_xpToNextLevel < 0 ? "Max" : ToString( m_curXP ) + "/" + ToString( m_xpToNextLevel );
		( (UIText*)xpLabel )->SetText( "XP: " + xpValue );
	}

	UIElement* hpLabel = g_game->GetUISystem()->GetUIElementByName( "hpLabel" );
	if ( hpLabel != nullptr )
	{
		int displayHealth = m_curHealth > 0 ? m_curHealth : 0;
		std::string hpValue = ToString( displayHealth ) + "/" + ToString( GetMaxHealth() );
		( (UIText*)hpLabel )->SetText( "HP: " + hpValue );
	}

	UIElement* hpBar = g_game->GetUISystem()->GetUIElementByName( "hpBar" );
	if ( hpBar != nullptr )
	{
		float hpPercent = (float)m_curHealth / (float)GetMaxHealth();
		hpBar->SetBoundingBoxPercent( Vec2( hpPercent, 1.f ) );

		if ( hpPercent > .5f )
		{
			hpBar->SetButtonAndLabelTint( Rgba8::GREEN );
		}
		else if ( hpPercent > .25f )
		{
			hpBar->SetButtonAndLabelTint( Rgba8::YELLOW );
		}
		else
		{
			hpBar->SetButtonAndLabelTint( Rgba8::RED );
		}
	}
}


//-----------------------------------------------------------------------------------------------
int Entity::GetTotalMaxHealth() const
{
	return GetMaxHealth() + m_equipMaxHealth;
}


//-----------------------------------------------------------------------------------------------
IntRange Entity::GetTotalAttackDamageRange() const
{
	return m_attackDamageRange + m_equipAttackDamageRange;
}


//-----------------------------------------------------------------------------------------------
IntRange Entity::GetTotalDefenseRange() const
{
	return m_defenseRange + m_equipDefenseRange;
}

//-----------------------------------------------------------------------------------------------
float Entity::GetTotalAttackRange() const
{
	return m_attackRange + m_equipAttackRange;
}


//-----------------------------------------------------------------------------------------------
float Entity::GetTotalAttackSpeed() const
{
	return m_attackSpeedModifier + m_equipAttackSpeedModifier;
}


//-----------------------------------------------------------------------------------------------
float Entity::GetTotalCritChance() const
{
	return m_critChance + m_equipCritChance;
}
