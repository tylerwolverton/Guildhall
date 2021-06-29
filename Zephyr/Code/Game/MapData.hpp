#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

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
	std::string name;
	Vec2 position = Vec2::ZERO;
	float yawDegrees = 0.f;

	ZephyrValueMap zephyrScriptInitialValues;
	std::vector<EntityVariableInitializer> zephyrEntityVarInits;

	// Portal specific variables
	// TODO: Move this to sub struct?
	std::string portalDestMap = "";
	Vec2		portalDestPos = Vec2::ZERO;
	float		portalDestYawOffset = 0.f;
};


//-----------------------------------------------------------------------------------------------
struct MapData
{
public:
	bool isValid = false;									// Game
	std::string mapName;									// Map
	std::string type = "InvalidType";						// World
	int version = -1;										
	IntVec2 dimensions = IntVec2::ZERO;						// TileMap

	std::vector<MapEntityDefinition> mapEntityDefs;			// Map
	std::vector<TileDefinition*> tileDefs;					// TileMap

	// Multiplayer TODO: Make this into an array
	Vec2 playerStartPos = Vec2::ZERO;
	float playerStartYaw = 0.f;

public:
	explicit MapData( const XmlElement& mapDefElem, const std::string& mapName, const std::string& defaultTileName );
	~MapData();
	
private:
	bool ParseMapDefinitionNode( const XmlElement& mapDefElem );
	bool ParseLegendNode( const XmlElement& mapDefElem, std::map<char, TileDefinition*>& legend, const std::string& defaultRegionName );
	bool ParseMapRowsNode( const XmlElement& mapDefElem, const std::map<char, TileDefinition*>& legend, const std::string& defaultRegionName );
	bool ParseEntitiesNode( const XmlElement& mapDefElem );
	void CreateMapEntityDefFromNode( const XmlElement& entityElem, const std::string& expectedType );
};
