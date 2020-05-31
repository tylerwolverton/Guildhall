#pragma once
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntRange.hpp"


//-----------------------------------------------------------------------------------------------
class MapGenStep_CellularAutomata : public MapGenStep
{
public:
	MapGenStep_CellularAutomata( const XmlElement& mapGenStepXmlElem );
	virtual void RunStepOnce( Map& map ) override;

private:
	TileDefinition* m_neighborTileType = nullptr;
	IntRange m_numNeighbors;
	IntRange m_neighborRadius;
};
