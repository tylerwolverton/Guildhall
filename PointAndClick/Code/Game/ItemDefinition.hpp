#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/EntityDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteAnimDefinition;
class SpriteAnimSetDefinition;
enum class eVerbState : int;

//-----------------------------------------------------------------------------------------------
class ItemDefinition : public EntityDefinition
{
	friend class Item;

public:
	explicit ItemDefinition( const XmlElement& itemDefElem );
	~ItemDefinition();

	std::string GetName() { return m_name; }
	NamedProperties* GetVerbEventProperties( eVerbState verbState );

	static ItemDefinition* GetItemDefinition( const std::string& itemName );

public:
	static std::map< std::string, ItemDefinition* > s_definitions;

private:
	std::map<eVerbState, NamedProperties*> m_verbPropertiesMap;
};
