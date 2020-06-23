#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class DialogueState
{
public:
	DialogueState( const XmlElement& dialogueStateDefElem );

	bool IsValid() const										{ return m_isValid; }
	int GetId() const											{ return m_id; }
	std::string GetName() const									{ return m_name; }

public:
	static std::map< std::string, DialogueState* > s_dialogueStateMap;

private:
	bool m_isValid = false;
	int m_id = -1;
	std::string m_name;
	std::vector<std::string> m_dialogueChoices;
	std::vector<std::string> m_targetDialogueStateIds;
};
