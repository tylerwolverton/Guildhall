#pragma once
#include "Engine/Math/Vec2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Actor;
class Item;


//-----------------------------------------------------------------------------------------------
class TriggerRegion
{
public:
	explicit TriggerRegion( const Vec2& position, float physicsRadius, const std::string& targetMap );
	~TriggerRegion();

	void AddRequiredItem( Item* item );
	
	const Vec2	 GetPosition() const					{ return m_position; };
	const float  GetPhysicsRadius() const				{ return m_physicsRadius; };

	void OnTriggerEnter( Actor* actor );

protected:
	Vec2				m_position = Vec2( -1.f, -1.f );
	float				m_physicsRadius = 0.f;

	std::vector<Item*>	m_requiredItems;
	std::string			m_targetMap;
};
