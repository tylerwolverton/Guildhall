#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

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

	SpriteDefinition* GetSpriteDef() const;

	void SetTexture( Texture* texture )									{ m_texture = texture; }

	void AddVerbState( eVerbState verbState, NamedProperties* properties );
	void RemoveVerbState( eVerbState verbState );

	// Events
	void HandleVerbAction( eVerbState verbState );

private:
	void UpdateAnimation();

protected:
	ItemDefinition*			m_itemDef = nullptr;
	float					m_cumulativeTime = 0.f;

	SpriteAnimDefinition*	m_curAnimDef = nullptr;
	Texture*				m_texture = nullptr;
};
