#include "Game/Scripting/ZephyrScript.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScript::ZephyrScript( const ZephyrScriptDefinition& scriptDef )
	: m_name( scriptDef.m_name )
{

}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::Update()
{
	// VM->Interpret( m_updateChunk );

	if ( !m_hasPrinted )
	{
		g_devConsole->PrintString( Stringf( "Hello, my name is %s!", m_name.c_str() ), Rgba8::MAGENTA );

		m_hasPrinted = true;
	}
}
