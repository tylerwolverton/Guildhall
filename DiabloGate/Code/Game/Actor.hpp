#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Time/Timer.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class SpriteAnimDefinition;
class Item;
class Map;


//-----------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( const EntityDefinition& entityDef, Map* curMap );
	virtual ~Actor() {}
};
