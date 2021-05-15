#pragma once
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class SpriteDefinition;


//-----------------------------------------------------------------------------------------------
class Item : public Entity
{
public:
	Item( const EntityDefinition& entityDef, Map* curMap );
	virtual ~Item() {}

	void RenderInUI() const;

	const SpriteDefinition* GetSpriteDef() const						{ return m_itemSpriteDef; };

private:
	const SpriteDefinition* m_itemSpriteDef = nullptr;
};
