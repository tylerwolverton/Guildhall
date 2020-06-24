#include "Game/DialogueState.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
DialogueState* DialogueState::GetDialogueState( const std::string& stateName )
{
	std::map< std::string, DialogueState* >::const_iterator  mapIter = DialogueState::s_dialogueStateMap.find( stateName );

	if ( mapIter == s_dialogueStateMap.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
std::map< std::string, DialogueState* > DialogueState::s_dialogueStateMap;


//-----------------------------------------------------------------------------------------------
DialogueState::DialogueState( const XmlElement& dialogueStateDefElem )
{
	m_name = ParseXmlAttribute( dialogueStateDefElem, "stateName", m_name );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "Dialogue state is missing name" );
		return;
	}

	m_introText = ParseXmlAttribute( dialogueStateDefElem, "introText", m_introText );

	const XmlElement* choiceElem = dialogueStateDefElem.FirstChildElement();
	while ( choiceElem != nullptr )
	{
		if ( !strcmp( choiceElem->Name(), "Choice" ) )
		{
			std::string text = ParseXmlAttribute( *choiceElem, "text", "" );
			std::string targetStateName = ParseXmlAttribute( *choiceElem, "targetState", "" );

			m_dialogueChoices.push_back( text );
			m_targetDialogueStateNames.push_back( targetStateName );
		}
		else if ( !strcmp( choiceElem->Name(), "Item" ) )
		{
			m_itemName = ParseXmlAttribute( *choiceElem, "name", m_itemName );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Dialogue state '%s' has unknown child node '%s'", m_name.c_str(), choiceElem->Name() ) );
		}

		choiceElem = choiceElem->NextSiblingElement();
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
DialogueState* DialogueState::GetNextDialogueStateFromChoice( const std::string choiceText ) const
{
	for ( int choiceIdx = 0; choiceIdx < (int)m_dialogueChoices.size(); ++choiceIdx )
	{
		if ( choiceText == m_dialogueChoices[choiceIdx] )
		{
			return GetDialogueState( m_targetDialogueStateNames[choiceIdx] );
		}
	}

	return nullptr;
}
