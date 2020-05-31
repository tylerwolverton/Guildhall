#pragma once
#include "Game/MapGenStep.hpp"


//-----------------------------------------------------------------------------------------------
class MapGenStep_Worm : public MapGenStep
{
public:
	MapGenStep_Worm( const XmlElement& mapGenStepXmlElem );
	virtual void RunStepOnce( Map& map ) override;

private:
	IntVec2 RollRandomCardinalDirection2D();

private:
	IntRange m_numWorms;
	IntRange m_wormLength;
};
