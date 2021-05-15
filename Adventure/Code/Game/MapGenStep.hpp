#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"


//-----------------------------------------------------------------------------------------------
class Map;
class TileDefinition;


//-----------------------------------------------------------------------------------------------
class MapGenStep
{
public: 
	MapGenStep( const XmlElement& mapGenStepXmlElem );
	virtual ~MapGenStep() {}
	
	void RunStep( Map& map );
	virtual void RunStepOnce( Map& map ) = 0;

public:
	static MapGenStep* CreateNewMapGenStep( const XmlElement& mapGenStepXmlElem );

	float m_chanceToRun = 0.f;
	IntRange m_numIterations;
	TileDefinition* m_setTileType = nullptr;
	TileDefinition* m_ifTileType = nullptr;
	FloatRange m_chancePerTile;
};
