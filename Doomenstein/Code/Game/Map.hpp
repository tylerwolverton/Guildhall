#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class MapDefinition;


//-----------------------------------------------------------------------------------------------
class Map
{
public:
	Map( std::string name, MapDefinition* mapDef );
	virtual ~Map();

	virtual void Load() = 0;
	virtual void Unload() = 0;

	virtual void UpdateMeshes() = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const = 0;

protected:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	IntVec2				m_dimensions;
};
