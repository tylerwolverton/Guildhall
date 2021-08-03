#include "Engine/ZephyrCore/ZephyrScript.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/ZephyrCore/ZephyrBytecodeChunk.hpp"
#include "Engine/ZephyrCore/ZephyrEntity.hpp"
#include "Engine/ZephyrCore/ZephyrScriptDefinition.hpp"
#include "Engine/ZephyrCore/ZephyrInterpreter.hpp"
#include "Engine/ZephyrCore/ZephyrEngineAPI.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScript::ZephyrScript( const ZephyrScriptDefinition& scriptDef, ZephyrEntity* parentEntity )
	: m_name( scriptDef.m_name )
	, m_scriptDef( scriptDef )
	, m_parentEntity( parentEntity )
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	m_globalBytecodeChunk = new ZephyrBytecodeChunk( *m_scriptDef.GetGlobalBytecodeChunk() );
	GUARANTEE_OR_DIE( m_globalBytecodeChunk != nullptr, "Global Bytecode Chunk was null" );
	
	m_curStateBytecodeChunk = m_scriptDef.GetFirstStateBytecodeChunk();
	m_stateBytecodeChunks = m_scriptDef.GetAllStateBytecodeChunks();

	// Initialize parentEntity in script
	m_globalBytecodeChunk->SetVariable( PARENT_ENTITY_NAME, ZephyrValue( (EntityId)m_parentEntity->GetId() ) );
}


//-----------------------------------------------------------------------------------------------
ZephyrScript::~ZephyrScript()
{
	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_globalBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::Update()
{
	if ( !IsScriptValid() )
	{
		EventArgs args;
		args.SetValue( "entity", (void*)m_parentEntity );
		args.SetValue( "text", "Script Error" );
		args.SetValue( "color", "red" );

		g_eventSystem->FireEvent( "PrintDebugText", &args );
		return;
	}

	// If this is the first update we need to call OnEnter explicitly
	if ( !m_hasEnteredStartingState )
	{
		if(	!m_isScriptObjectValid )
		{
			return;
		}

		m_hasEnteredStartingState = true;

		FireEvent( "OnEnter" );
	}

	FireEvent( "OnUpdate" );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::UnloadScript()
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_stateBytecodeChunks.clear();
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScript::FireEvent( const std::string& eventName, EventArgs* args )
{
	if ( !m_scriptDef.IsValid() )
	{
		return false;
	}

	EventArgs eventArgs;
	if ( args == nullptr )
	{
		args = &eventArgs;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk != nullptr )
	{
		ZephyrValueMap* stateVariables = nullptr;
		if ( m_curStateBytecodeChunk != nullptr )
		{
			stateVariables = m_curStateBytecodeChunk->GetUpdateableVariables();
		}

		m_parentEntity->AddGameEventParams( args );
		
		ZephyrInterpreter::InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args, stateVariables );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::ChangeState( const std::string& targetState )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* targetStateBytecodeChunk = GetStateBytecodeChunk( targetState );
	if ( targetStateBytecodeChunk == nullptr 
		 || targetStateBytecodeChunk == m_curStateBytecodeChunk )
	{
		// State doesn't exist, should be reported by compiler to avoid flooding with errors here
		// Or the state change is a no-op
		return;
	}

	FireEvent( "OnExit" );
	
	m_curStateBytecodeChunk = targetStateBytecodeChunk;
	// Initialize state variables each time the state is entered
	ZephyrInterpreter::InterpretStateBytecodeChunk( *m_curStateBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, m_curStateBytecodeChunk->GetUpdateableVariables() );

	FireEvent( "OnEnter" );
	m_hasEnteredStartingState = true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::InterpretGlobalBytecodeChunk()
{
	ZephyrInterpreter::InterpretStateBytecodeChunk( *m_globalBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	
	// Initialize default state variables
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrInterpreter::InterpretStateBytecodeChunk( *m_curStateBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, m_curStateBytecodeChunk->GetUpdateableVariables() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::InitializeGlobalVariables( const ZephyrValueMap& intialValues )
{
	if ( m_globalBytecodeChunk == nullptr )
	{
		return;
	}

	ZephyrValueMap* globalVariables = m_globalBytecodeChunk->GetUpdateableVariables();
	if ( globalVariables == nullptr )
	{
		return;
	}

	for ( auto const& initialValue : intialValues )
	{
		const auto globalVarIter = globalVariables->find( initialValue.first );
		if ( globalVarIter == globalVariables->end() )
		{
			g_devConsole->PrintError( Stringf( "Cannot initialize nonexistent variable '%s' in script '%s'", initialValue.first.c_str(), m_name.c_str() ) );
			m_isScriptObjectValid = false;
			continue;
		}

		(*globalVariables)[initialValue.first] = initialValue.second;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits )
{
	m_entityVarInits.insert( m_entityVarInits.begin(), entityVarInits.begin(), entityVarInits.end() );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScript::IsScriptValid() const
{
	return m_isScriptObjectValid && m_scriptDef.IsValid();
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::SetScriptObjectValidity( bool isValid )
{
	m_isScriptObjectValid = isValid;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrScript::GetGlobalVariable( const std::string& varName )
{
	ZephyrValue val( ERROR_ZEPHYR_VAL );

	m_globalBytecodeChunk->TryToGetVariable( varName, val );

	return val;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
	m_globalBytecodeChunk->SetVariable( varName, value );
}


////-----------------------------------------------------------------------------------------------
//void ZephyrScript::SetGlobalVec2Member( const std::string& varName, const std::string& memberName, const ZephyrValue& value )
//{
//	// Already checked to make sure this is a valid member
//	m_globalBytecodeChunk->SetVec2Member( varName, memberName, value );
//}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::InitializeEntityVariables()
{
	ZephyrValueMap validEntities;
	
	for ( const auto& entityVarInit : m_entityVarInits )
	{
		ZephyrEntity* entity = g_zephyrAPI->GetEntityByName( entityVarInit.entityName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Error defining entity variable '%s' in zephyr script. Entity with name '%s' can not be found", entityVarInit.varName.c_str(), entityVarInit.entityName.c_str() ) );
			m_isScriptObjectValid = false;
			continue;
		}

		validEntities[entityVarInit.varName] = entity->GetId();
	}

	InitializeGlobalVariables( validEntities );
}


//-----------------------------------------------------------------------------------------------
const ZephyrBytecodeChunk* ZephyrScript::GetBytecodeChunkByName( const std::string& chunkName ) const
{
	if ( m_globalBytecodeChunk->GetName() == chunkName )
	{
		return m_globalBytecodeChunk;
	}

	// Check for global functions
	for ( const auto& eventPair : m_globalBytecodeChunk->GetEventBytecodeChunks() )
	{
		if ( eventPair.second->GetName() == chunkName )
		{
			return eventPair.second;
		}
	}

	for ( const auto& statePair : m_stateBytecodeChunks )
	{
		if ( statePair.second->GetName() == chunkName )
		{
			return statePair.second;
		}

		for ( const auto& eventPair : statePair.second->GetEventBytecodeChunks() )
		{
			if ( eventPair.second->GetName() == chunkName )
			{
				return eventPair.second;
			}
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::OnEvent( EventArgs* args )
{
	std::string eventName = args->GetValue( "eventName", "" );

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk != nullptr )
	{
		ZephyrValueMap* stateVariables = nullptr;
		if ( m_curStateBytecodeChunk != nullptr )
		{
			stateVariables = m_curStateBytecodeChunk->GetUpdateableVariables();
		}

		ZephyrInterpreter::InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args, stateVariables );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScript::GetStateBytecodeChunk( const std::string& stateName )
{
	ZephyrBytecodeChunkMap::const_iterator  mapIter = m_stateBytecodeChunks.find( stateName );

	if ( mapIter == m_stateBytecodeChunks.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScript::GetEventBytecodeChunk( const std::string& eventName )
{
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrBytecodeChunkMap::const_iterator  mapIter = m_curStateBytecodeChunk->GetEventBytecodeChunks().find( eventName );

		if ( mapIter != m_curStateBytecodeChunk->GetEventBytecodeChunks().cend() )
		{
			return mapIter->second;
		}
	}

	ZephyrBytecodeChunkMap::const_iterator mapIter = m_globalBytecodeChunk->GetEventBytecodeChunks().find( eventName );

	if ( mapIter != m_globalBytecodeChunk->GetEventBytecodeChunks().cend() )
	{
		return mapIter->second;
	}

	return nullptr;
}
