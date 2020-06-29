#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimationSetDefinition;


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem );

	bool		IsValid() const											{ return m_isValid; }
	std::string GetName() const											{ return m_name; }
	std::string GetType() const											{ return m_type; }
	float		GetWalkSpeed() const									{ return m_walkSpeed; }
	Vec2		GetVisualSize() const									{ return m_visualSize; }

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_name;
	std::string		m_type;
	float			m_physicsRadius = 0.f;
	float			m_height = 0.f;
	float			m_eyeHeight = 0.f;
	float			m_mass = 1.f;
	float			m_walkSpeed = 0.f;

	Vec2			m_visualSize = Vec2::ZERO;
	eBillboardStyle m_billboardStyle = eBillboardStyle::CAMERA_FACING_INVALID;

	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
};
