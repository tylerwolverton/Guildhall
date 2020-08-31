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
class ActorDefinition : public EntityDefinition
{
	friend class Actor;

public:
	explicit ActorDefinition( const XmlElement& actorDefElem );
	~ActorDefinition();

	std::string GetName() { return m_name; }
	//SpriteAnimDefinition* GetSpriteAnimDef( std::string animName );

	static ActorDefinition* GetActorDefinition( std::string actorName );
	
public:
	static std::map< std::string, ActorDefinition* > s_definitions;

private:	
	float m_walkSpeed = 0.f;
	float m_flySpeed = 0.f;
	float m_swimSpeed = 0.f;
	
	SpriteAnimSetDefinition* m_spriteAnimSetDef = nullptr;
};
