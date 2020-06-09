#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class ItemDefinition;
class SpriteDefinition;
class SpriteAnimDefinition;


//-----------------------------------------------------------------------------------------------
class Item : public Entity
{
public:
	Item( const Vec2& position, ItemDefinition* itemDef );
	~Item();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

	void SetIsInPlayerInventory( bool isInPlayerInventory )										{ m_isInPlayerInventory = isInPlayerInventory; }

	SpriteDefinition* GetSpriteDef() const;

private:
	void UpdateAnimation();

protected:
	ItemDefinition*			m_itemDef = nullptr;
	float					m_cumulativeTime = 0.f;

	bool					m_isInPlayerInventory = false;

	SpriteAnimDefinition*	m_curAnimDef = nullptr;
};
