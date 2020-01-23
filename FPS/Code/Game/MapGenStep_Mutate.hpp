#pragma once
#include "Game/MapGenStep.hpp"


//-----------------------------------------------------------------------------------------------
class MapGenStep_Mutate : public MapGenStep
{
public:
	MapGenStep_Mutate( const XmlElement& mapGenStepXmlElem );
	virtual void RunStepOnce( Map& map ) override;
};
