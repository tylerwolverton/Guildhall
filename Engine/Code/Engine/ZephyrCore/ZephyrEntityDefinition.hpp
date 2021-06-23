#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrEntityDefinition
{
public:
	explicit ZephyrEntityDefinition( const XmlElement& entityDefElem );
	virtual ~ZephyrEntityDefinition() {}

	void									ReloadZephyrScriptDefinition();
		
	ZephyrScriptDefinition*					GetZephyrScriptDefinition() const							{ return m_zephyrScriptDef; }
	ZephyrValueMap							GetZephyrScriptInitialValues() const						{ return m_zephyrScriptInitialValues; }
	std::vector<EntityVariableInitializer>	GetZephyrEntityVarInits() const								{ return m_zephyrEntityVarInits; }

protected:
	bool									m_isScriptValid = false;

	std::string								m_zephyrScriptName;
	ZephyrScriptDefinition*					m_zephyrScriptDef = nullptr;
	ZephyrValueMap							m_zephyrScriptInitialValues;
	std::vector<EntityVariableInitializer>  m_zephyrEntityVarInits;
};
