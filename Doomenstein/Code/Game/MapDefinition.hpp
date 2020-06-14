#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/MapRegionTypeDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition;
class MapRegionTypeDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
class MapDefinition
{
	friend class Map;
	friend class TileMap;

public:
	explicit MapDefinition( const XmlElement& mapDefElem, const std::string& name );
	~MapDefinition();

	std::string GetName()													{ return m_name; }

	Vec2 GetPlayerStartPos() const											{ return m_playerStartPos; }
	float GetPlayerStartYaw() const											{ return m_playerStartYaw; }

	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	bool ParseMapDefinitionNode( const XmlElement& mapDefElem );
	bool ParseLegendNode( const XmlElement& mapDefElem );
	bool ParseMapRowsNode( const XmlElement& mapDefElem );
	bool ParseEntitiesNode( const XmlElement& mapDefElem );

private:
	std::string m_name;
	std::string m_type = "InvalidType";
	int m_version = -1;
	IntVec2 m_dimensions = IntVec2::ZERO;

	std::map<char, std::string> m_legend;
	std::vector<MapRegionTypeDefinition*> m_regionTypeDefs;

	// Multiplayer TODO: Make this into an array
	Vec2 m_playerStartPos = Vec2::ZERO;
	float m_playerStartYaw = 0.f;
};
