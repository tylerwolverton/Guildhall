#include "Engine/ZephyrCore/ZephyrEntity.hpp"
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
#include "Engine/ZephyrCore/ZephyrScriptDefinition.hpp"
#include "Engine/ZephyrCore/ZephyrEntityDefinition.hpp"
#include "Engine/ZephyrCore/ZephyrScript.hpp"


//-----------------------------------------------------------------------------------------------
EntityId ZephyrEntity::s_nextEntityId = 1000;


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::CreateZephyrScript( const ZephyrEntityDefinition& entityDef )
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
ZephyrEntity::ZephyrEntity( const ZephyrEntityDefinition& entityDef )
	: m_entityDef( entityDef )
{
	m_id = s_nextEntityId++;
}


//-----------------------------------------------------------------------------------------------
ZephyrEntity::~ZephyrEntity()
{
	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_scriptObj );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( m_scriptObj != nullptr )
	{
		m_scriptObj->Update();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::Die()
{	
	if ( IsScriptValid() )
	{
		EventArgs args;
		m_scriptObj->FireEvent( "OnDie", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::FireSpawnEvent()
{
	if ( IsScriptValid() )
	{
		EventArgs args;
		args.SetValue( "EntityId", GetId() );
		args.SetValue( "EntityName", GetName() );

		m_scriptObj->FireEvent( "OnSpawn", &args );
	}
}


//-----------------------------------------------------------------------------------------------
bool ZephyrEntity::FireScriptEvent( const std::string& eventName, EventArgs* args )
{
	if ( !IsScriptValid() )
	{
		return false;
	}

	return m_scriptObj->FireEvent( eventName, args );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::ChangeZephyrScriptState( const std::string& targetState )
{
	if ( !IsScriptValid() )
	{
		g_devConsole->PrintWarning( Stringf( "Tried to change state of ZephyrEntity: %s to %s, but it doesn't have a valid script", m_name.c_str(), targetState.c_str() ) );
		return;
	}

	m_scriptObj->ChangeState( targetState );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::UnloadZephyrScript()
{
	if ( m_scriptObj == nullptr )
	{
		return;
	}

	m_scriptObj->UnloadScript();
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::ReloadZephyrScript()
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
void ZephyrEntity::InitializeScriptValues( const ZephyrValueMap& initialValues )
{
	if ( IsScriptValid() )
	{
		m_scriptObj->InitializeGlobalVariables( initialValues );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits )
{
	if ( IsScriptValid() )
	{
		m_scriptObj->SetEntityVariableInitializers( entityVarInits );
	}
}


//-----------------------------------------------------------------------------------------------
const ZephyrBytecodeChunk* ZephyrEntity::GetBytecodeChunkByName( const std::string& chunkName ) const
{
	if ( m_scriptObj == nullptr )
	{
		return nullptr;
	}

	return m_scriptObj->GetBytecodeChunkByName( chunkName );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrEntity::GetGlobalVariable( const std::string& varName )
{
	if ( !IsScriptValid() )
	{
		return ZephyrValue( ERROR_ZEPHYR_VAL );
	}
	
	return m_scriptObj->GetGlobalVariable( varName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
	if ( !IsScriptValid() )
	{
		return;
	}
	
	m_scriptObj->SetGlobalVariable( varName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::SetGlobalVec2Variable( const std::string& varName, const std::string& memberName, const ZephyrValue& value )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_scriptObj->SetGlobalVec2Variable( varName, memberName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::InitializeZephyrEntityVariables()
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_scriptObj->InitializeEntityVariables();
}


//-----------------------------------------------------------------------------------------------
bool ZephyrEntity::IsScriptValid() const
{
	if ( m_scriptObj == nullptr )
	{
		return false;
	}

	return m_scriptObj->IsScriptValid();
}


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::SetScriptObjectValidity( bool isValid )
{
	if ( m_scriptObj == nullptr )
	{
		return;
	}

	return m_scriptObj->SetScriptObjectValidity( isValid );
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrEntity::GetScriptName() const
{
	if ( m_scriptObj == nullptr )
	{
		return "unknown";
	}

	return m_scriptObj->GetScriptName();
}
