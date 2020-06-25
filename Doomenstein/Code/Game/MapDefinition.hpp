#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/MapRegionTypeDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition;
class EntityDefinition;
class MapRegionTypeDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
struct MapEntityDefinition
{
	EntityDefinition* entityDef = nullptr;
	Vec2 position = Vec2::ZERO;
	float yawDegrees = 0.f;
};


//-----------------------------------------------------------------------------------------------
class MapDefinition
{
	friend class Map;
	friend class TileMap;

public:
	explicit MapDefinition( const XmlElement& mapDefElem, const std::string& mapName, const std::string& defaultRegionName );
	~MapDefinition();

	bool IsValid() const													{ return m_isValid; }
	std::string GetName() const												{ return m_name; }
	std::string GetType() const												{ return m_type; }

	Vec2 GetPlayerStartPos() const											{ return m_playerStartPos; }
	float GetPlayerStartYaw() const											{ return m_playerStartYaw; }
	std::vector<MapEntityDefinition> GetMapEntityDefs() const				{ return m_mapEntityDefs; }

	// Static methods
	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	bool ParseMapDefinitionNode( const XmlElement& mapDefElem );
	bool ParseLegendNode( const XmlElement& mapDefElem, std::map<char, MapRegionTypeDefinition*>& legend );
	bool ParseMapRowsNode( const XmlElement& mapDefElem, const std::map<char, MapRegionTypeDefinition*>& legend );
	bool ParseEntitiesNode( const XmlElement& mapDefElem );

private:
	bool m_isValid = false;
	std::string m_name;
	std::string m_defaultRegionTypeStr;
	std::string m_type = "InvalidType";
	int m_version = -1;
	IntVec2 m_dimensions = IntVec2::ZERO;

	std::vector<MapEntityDefinition> m_mapEntityDefs;
	std::vector<MapRegionTypeDefinition*> m_regionTypeDefs;

	// Multiplayer TODO: Make this into an array
	Vec2 m_playerStartPos = Vec2::ZERO;
	float m_playerStartYaw = 0.f;
};
