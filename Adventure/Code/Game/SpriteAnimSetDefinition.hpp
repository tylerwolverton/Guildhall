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
	friend class EntityDefinition;

public:
	SpriteAnimSetDefinition( RenderContext& renderer, const XmlElement& spriteAnimSetDefElem );
	~SpriteAnimSetDefinition();

	static SpriteAnimDefinition* GetSpriteAnimDefinitionByName( const std::string& name );

private:
	static std::map< std::string, SpriteAnimDefinition* > s_spriteAnimDefMapByName;

	SpriteSheet* m_spriteSheet = nullptr;
};
