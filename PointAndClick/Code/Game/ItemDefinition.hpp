#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/EntityDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteAnimDefinition;
class SpriteAnimSetDefinition;


//-----------------------------------------------------------------------------------------------
class ItemDefinition : public EntityDefinition
{
	friend class Item;

public:
	explicit ItemDefinition( const XmlElement& itemDefElem );
	~ItemDefinition();

	std::string GetName() { return m_name; }
	SpriteAnimDefinition* GetSpriteAnimDef( const std::string& animName );

	static ItemDefinition* GetItemDefinition( const std::string& itemName );

public:
	static std::map< std::string, ItemDefinition* > s_definitions;

private:
	SpriteAnimSetDefinition* m_spriteAnimSetDef = nullptr;
};
