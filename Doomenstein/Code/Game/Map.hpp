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

	virtual void UpdateMeshes() = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const = 0;

protected:
	std::string			m_name;
	MapDefinition*      m_mapDef;

	int					m_width = 0;
	int					m_height = 0;
};
