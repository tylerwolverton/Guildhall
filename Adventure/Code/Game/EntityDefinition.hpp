#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"

#include <string>


class SpriteSheet;


class EntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem );

	std::string GetName()											{ return m_name; }
	std::string GetFaction()										{ return m_faction; }

	bool CanWalk()													{ return m_canWalk; }
	bool CanFly()													{ return m_canFly; }
	bool CanSwim()													{ return m_canSwim; }

protected:
	std::string		m_name;
	std::string		m_faction;
	float			m_physicsRadius = 0.f;
	AABB2			m_localDrawBounds = AABB2::ONE_BY_ONE;
	int				m_maxHealth = 1;
	int				m_startHealth = 1;
	SpriteSheet*	m_spriteSheet = nullptr;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;

	bool			m_canWalk = false;
	bool			m_canFly = false;
	bool			m_canSwim = false;

};

