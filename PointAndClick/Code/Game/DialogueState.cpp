#include "Game/DialogueState.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, DialogueState* > DialogueState::s_dialogueStateMap;


//-----------------------------------------------------------------------------------------------
DialogueState::DialogueState( const XmlElement& dialogueStateDefElem )
{
	m_id = ParseXmlAttribute( dialogueStateDefElem, "id", m_id );
	/*if ( m_id == -1 )
	{
		g_devConsole->PrintError( "Dialogue state is missing id" );
		return;
	}*/

	m_name = ParseXmlAttribute( dialogueStateDefElem, "stateName", m_name );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "Dialogue state is missing name" );
		return;
	}

	const XmlElement* choiceElem = dialogueStateDefElem.FirstChildElement();
	if ( choiceElem != nullptr )
	{
		if ( !strcmp( choiceElem->Name(), "Choice" ) )
		{
			std::string text = ParseXmlAttribute( *choiceElem, "text", "" );
			std::string targetStateName = ParseXmlAttribute( *choiceElem, "targetState", "" );

			m_dialogueChoices.push_back( text );
			m_targetDialogueStateIds.push_back( targetStateName );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Dialogue state '%s' has unknown child node '%s'", m_name.c_str(), choiceElem->Name() ) );
		}

		choiceElem = choiceElem->NextSiblingElement();
	}

	/*s_dialogueStates.reserve( m_id );
	s_dialogueStates[m_id] = *this;*/
	m_isValid = true;
}
