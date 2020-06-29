#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteAnimDefinition;
class SpriteSheet;
class RenderContext;
class Transform;
class Camera;


//-----------------------------------------------------------------------------------------------
class SpriteAnimationSetDefinition
{
	friend class ActorDefinition;
	friend class ItemDefinition;

public:
	SpriteAnimationSetDefinition( SpriteSheet* spriteSheet, const XmlElement& spriteAnimSetDefElem );
	~SpriteAnimationSetDefinition();

	SpriteAnimDefinition* GetSpriteAnimationDefForDirection( const Vec2& entityPos, float entityOrientationDegrees, const Camera& camera );

private:
	std::map< std::string, SpriteAnimDefinition* > m_spriteAnimDefMapByName;

	std::string m_name;
	SpriteSheet* m_spriteSheet = nullptr;
	SpriteAnimDefinition* m_frontAnimDef = nullptr;
};
