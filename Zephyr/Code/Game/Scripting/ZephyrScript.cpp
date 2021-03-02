#include "Game/Scripting/ZephyrScript.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"
#include "Game/Scripting/ZephyrInterpreter.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScript::ZephyrScript( const ZephyrScriptDefinition& scriptDef, Entity* parentEntity )
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

	RegisterScriptEvents( m_globalBytecodeChunk );
	RegisterScriptEvents( m_curStateBytecodeChunk );
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
	if ( !m_hasUpdated )
	{
		if(	!m_isScriptObjectValid )
		{
			return;
		}

		m_hasUpdated = true;

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

	UnRegisterScriptEvents( m_curStateBytecodeChunk );
	UnRegisterScriptEvents( m_scriptDef.GetGlobalBytecodeChunk() );

	m_stateBytecodeChunks.clear();
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::FireEvent( const std::string& eventName, EventArgs* args )
{
	EventArgs eventArgs;
	if ( args == nullptr )
	{
		args = &eventArgs;
	}

	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk != nullptr )
	{
		ZephyrValueMap* stateVariables = nullptr;
		if ( m_curStateBytecodeChunk != nullptr )
		{
			stateVariables = m_curStateBytecodeChunk->GetUpdateableVariables();
		}

		Map* parentMap = m_parentEntity->GetMap();
		if ( parentMap != nullptr )
		{
			args->SetValue( "mapName", parentMap->GetName() );
		}

		ZephyrInterpreter::InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args, stateVariables );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::ChangeState( const std::string& targetState )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* targetStateBytecodeChunk = GetStateBytecodeChunk( targetState );
	if ( targetStateBytecodeChunk == nullptr )
	{
		// State doesn't exist, should be reported by compiler to avoid flooding with errors here
		return;
	}

	FireEvent( "OnExit" );

	UnRegisterScriptEvents( m_curStateBytecodeChunk );

	m_curStateBytecodeChunk = targetStateBytecodeChunk;

	RegisterScriptEvents( m_curStateBytecodeChunk );

	FireEvent( "OnEnter" );
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


//-----------------------------------------------------------------------------------------------
void ZephyrScript::SetGlobalVec2Variable( const std::string& varName, const std::string& memberName, const ZephyrValue& value )
{
	// Already checked to make sure this is a valid member
	m_globalBytecodeChunk->SetVec2Variable( varName, memberName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::InitializeEntityVariables()
{
	ZephyrInterpreter::InterpretStateBytecodeChunk( *m_globalBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );

	ZephyrValueMap validEntities;
	
	for ( const auto& entityVarInit : m_entityVarInits )
	{
		Entity* entity = g_game->GetEntityByName( entityVarInit.entityName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Error defining entity variable '%s' in zephyr script. Entity with name '%s' can not be found", entityVarInit.varName.c_str(), entityVarInit.entityName.c_str() ) );
			m_isScriptObjectValid = false;
			continue;
		}

		validEntities[entityVarInit.varName] = entity->GetId();
	}

	// Initialize this script's parent entity
	validEntities[PARENT_ENTITY_NAME] = m_parentEntity->GetId();

	InitializeGlobalVariables( validEntities );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::RegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk )
{
	if ( bytecodeChunk == nullptr )
	{
		return;
	}

	for ( auto chunk : bytecodeChunk->GetEventBytecodeChunks() )
	{
		g_eventSystem->RegisterMethodEvent( chunk.first, "", eUsageLocation::EVERYWHERE, this, &ZephyrScript::OnEvent );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::UnRegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk )
{
	if ( bytecodeChunk == nullptr )
	{
		return;
	}

	for ( auto chunk : bytecodeChunk->GetEventBytecodeChunks() )
	{
		g_eventSystem->DeRegisterMethodEvent( chunk.first, this, &ZephyrScript::OnEvent );
	}
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
