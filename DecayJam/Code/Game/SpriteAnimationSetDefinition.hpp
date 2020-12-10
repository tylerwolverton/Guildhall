#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteAnimDefinition;
class SpriteSheet;
class RenderContext;
class Transform;
class Camera;


//-----------------------------------------------------------------------------------------------
struct DirectionAnimation
{
public:
	Vec2 facingDirection = Vec2::ZERO;
	SpriteAnimDefinition* animDef = nullptr;

public: 
	~DirectionAnimation() { PTR_SAFE_DELETE( animDef ); }
};


//-----------------------------------------------------------------------------------------------
class SpriteAnimationSetDefinition
{
	friend class ActorDefinition;
	friend class ItemDefinition;

public:
	SpriteAnimationSetDefinition( SpriteSheet* spriteSheet, const XmlElement& spriteAnimSetDefElem, float defaultFPS );
	~SpriteAnimationSetDefinition();

	SpriteAnimDefinition* GetSpriteAnimationDefForDirection( const Vec2& direction );

private:
	void AddDirectionAnimation( const std::string& animName, const Vec2& facingDir, const XmlElement& spriteAnimSetDefElem );

private:
	std::map< std::string, DirectionAnimation* > m_directionSpriteAnims;

	std::string m_name;
	float m_defaultFPS = 1.f;
	SpriteSheet* m_spriteSheet = nullptr;
};
