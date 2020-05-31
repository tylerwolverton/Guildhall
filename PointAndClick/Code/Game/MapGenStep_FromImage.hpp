#pragma once
#include "Game/MapGenStep.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class MapGenStep_FromImage : public MapGenStep
{
public:
	explicit MapGenStep_FromImage( const XmlElement& mapGenStepXmlElem );
	virtual void RunStepOnce( Map& map ) override;

private:
	std::string m_imageFilePath;
	IntRange m_numRotations;
	FloatRange m_alignmentX;
	FloatRange m_alignmentY;
	float m_chanceToMirror = 0.f;
};
