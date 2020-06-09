#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteAnimDefinition;
class SpriteSheet;
class RenderContext;


//-----------------------------------------------------------------------------------------------
class SpriteAnimSetDefinition
{
	friend class ActorDefinition;
	friend class ItemDefinition;

public:
	SpriteAnimSetDefinition( RenderContext& renderer, const XmlElement& spriteAnimSetDefElem );
	~SpriteAnimSetDefinition();

private:
	std::map< std::string, SpriteAnimDefinition* > m_spriteAnimDefMapByName;

	SpriteSheet* m_spriteSheet = nullptr;
};